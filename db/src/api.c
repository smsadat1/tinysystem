// cache api

#include "tinycache.h"


bool tc_set(Cache* c, command* str) {
    return insert(
        c,
        str->key,
        str->value, str->value_len,
        TC_NO_EXPIRY
    );
}

void* tc_get(Cache* c, command* str) {
    
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return NULL;
    }

    if(ce->expires_at != 0 &&
    ce->expires_at <= (uint64_t)time(NULL)) {
        // lazy deletion
        delete(c, str->key);
        pthread_mutex_unlock(&c->lock);
        return NULL;
    }

    void* value = malloc(ce->value_len);
    if (value == NULL && ce->value_len > 0) {
        pthread_mutex_unlock(&c->lock);
        return NULL;
    }

    if (ce->value_len > 0) {
        memcpy(value, ce->value, ce->value_len);
    }

    // update LRU
    lru_update_lock(c, ce);

    pthread_mutex_unlock(&c->lock);

    return value;
}

bool tc_delete(Cache* c, command* str) {

    bool result = false;

    pthread_mutex_lock(&c->lock);
    result = delete(c, str->key);
    pthread_mutex_unlock(&c->lock);
    
    return result;
}

bool tc_exists(Cache* c, command* str) {
    
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    if(ce->expires_at != 0 &&
    ce->expires_at <= (uint64_t)time(NULL)) {
        // lazy deletion
        delete(c, str->key);
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    // update LRU
    lru_update_lock(c, ce);

    pthread_mutex_unlock(&c->lock);
    return true;
}

bool tc_expire(Cache* c, command* str) {

    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    if(ce->expires_at != 0 &&
    ce->expires_at <= (uint64_t)time(NULL)) {
        // lazy deletion
        delete(c, str->key);
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    // can set expiry if already not set 
    if (!ce->expiry) {
        ce->expires_at = time(NULL) + str->ttl;
        ce->expiry = true;
        pthread_mutex_unlock(&c->lock);
        return true;
    }

    if (ce->expiry) {
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    // should not reach here
    return false;
}

uint32_t tc_ttl(Cache* c, command* str) {
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return 0;
    }

    if(ce->expires_at != 0 &&
    ce->expires_at <= (uint64_t)time(NULL)) {
        // lazy deletion
        delete(c, str->key);
        pthread_mutex_unlock(&c->lock);
        return 0;
    }

    if (ce->expires_at == 0) {
       pthread_mutex_unlock(&c->lock);
       return 0;
    }

    uint32_t remaining = (uint32_t)(ce->expires_at - (uint64_t)time(NULL));
    pthread_mutex_unlock(&c->lock);
    return remaining;
}

void tc_stat(Cache* c) {
    pthread_mutex_lock(&c->lock);
    
    printf("Total entries: %lu\n", c->entries);
    printf("Memory used (bytes): %lu/%lu\n", c->mem_used, c->mem_max);

    if(c->policy == EVICTION_LRU) {
        puts("Eviction policy: LRU");
    } 
    else if (c->policy == EVICTION_LFU)
    {
        puts("Eviction policy: LFU");
    }

    pthread_mutex_unlock(&c->lock);
}