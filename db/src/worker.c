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
    tc_log(TC_INFO, "Max set memory(mb): %d\n", config->max_mem_mb);

    while (true)
    {
        sleep(TC_INTERVAL_SEC);
        
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
}