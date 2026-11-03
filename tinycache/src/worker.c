#include "tinycache.h"


// thread safe
void remove_entry(Cache* c, cache_entry* ce) {
   
    pthread_mutex_lock(&c->lock);
    delete(c, ce->key);
    pthread_mutex_unlock(&c->lock);
}

static cache_entry* choose_victim_lock(Cache* c) {
    
    switch (c->policy) {
        case EVICTION_LRU:
            return c->lru_tail;
        case EVICTION_LFU:
            return find_lfu_victim(c);
        default:
            return NULL;
    }
}

void* worker(void* arg) {
    
    WorkerConfig* config = (WorkerConfig*)arg;

    if(config->policy == EVICTION_LRU) {
        tc_log(TC_INFO, "Memory eviction policy: LRU");
    } 
    else if (config->policy == EVICTION_LFU) {
        tc_log(TC_INFO, "Memory eviction policy: LFU");
    }
    tc_log(TC_INFO, "Max set memory(mb): %d", config->max_mem_mb);

    while (!atomic_load(&config->cache->shutdown))
    {
        struct timespec deadline;
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        deadline.tv_sec += TC_INTERVAL_SEC;
        int rc = 0;

        tc_log(TC_DEBUG, "Eviction worker checking memory");

        pthread_mutex_lock(&config->cache->worker_lock);
        while (!atomic_load(&config->cache->shutdown) && rc != ETIMEDOUT)
        {
            rc = pthread_cond_timedwait(
                &config->cache->worker_cond,
                &config->cache->worker_lock,
                &deadline
            );
        }
        pthread_mutex_unlock(&config->cache->worker_lock);

        // exit during shutdown
        if(atomic_load(&config->cache->shutdown))
            break;

        // thread safe eviction
        pthread_mutex_lock(&config->cache->lock);

        // eviction starts when max memory usage exceed
        if(config->cache->mem_used > config->cache->mem_max) {
            
            puts("Warning: Memory limit exceeded!\nPerforming cache eviction");

            while (config->cache->mem_used > config->cache->mem_max)
            {
                cache_entry* victim = choose_victim_lock(config->cache);
                if(victim == NULL) {
                    break;
                }
                remove_entry(config->cache, victim);
            }
        }

        pthread_mutex_unlock(&config->cache->lock);
    }

    // exit
    tc_log(TC_DEBUG, "Exiting eviction worker");
    return NULL;
}

void exit_worker(Cache* cache, pthread_t* worker_thread) {
    atomic_store(&cache->shutdown, true);

    pthread_mutex_lock(&cache->worker_lock);
    pthread_cond_broadcast(&cache->worker_cond);
    pthread_mutex_unlock(&cache->worker_lock);

    int err = pthread_join(*worker_thread, NULL);  
    if (err != 0) {
        tc_log(TC_ERROR, "pthread_join failed: %d (%s)\n",
                err, strerror(err));
        abort();
    }
}