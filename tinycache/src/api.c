// cache api

#include "tinycache.h"


bool tc_set(Cache* c, command* str) {
    
    if (c == NULL || str == NULL ||
        str->key == NULL ||
        (str->value == NULL && str->value_len > 0)) {
        return false;
    }
    
    // first add entry on disk
    if(!tc_add_record(str->key, str->value, str->value_len)) 
        return false;

    // then add entry in cache
    return insert(
        c,
        str->key,
        str->value, str->value_len
    );
}

void* tc_get(Cache* c, command* str, size_t* value_len) {
    
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);

    if(ce != NULL) {
        
        void* value = malloc(ce->value_len);
        if (value == NULL && ce->value_len > 0) {
            pthread_mutex_unlock(&c->lock);
            return NULL;
        }

        if (ce->value_len > 0) {
            memcpy(value, ce->value, ce->value_len);
            *value_len = ce->value_len;
        }
        // update LRU
        lru_update_lock(c, ce);

        pthread_mutex_unlock(&c->lock);

        return value;
    }

    pthread_mutex_unlock(&c->lock);

    // Cache miss: retrieve from persistent storage
    void* value = NULL;
    size_t disk_value_len = 0;
    
    if(!tc_get_record(str->key, &value, &disk_value_len)) {
        // does not exist on disk record either
        return NULL;
    }

    // Populate cache after cache miss
    tc_log(TC_DEBUG, "Found in disk (Key: %s | Value: %s)", str->key, (char*)value);
    tc_log(TC_DEBUG, "Key: %s Adding to cache (cache miss)", str->key);
    insert(c, str->key, value, disk_value_len);
    
    *value_len = disk_value_len;
    return value;
}

bool tc_delete(Cache* c, command* str) {

    bool result = false;

    // first remove entry from cache
    pthread_mutex_lock(&c->lock);
    result = delete(c, str->key);
    pthread_mutex_unlock(&c->lock);
    
    // then remove entry from disk
    if(!tc_delete_record(str->key))
        result = false;

    return result;
}

bool tc_exists(Cache* c, command* str) {
    
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        tc_log(TC_DEBUG, "Key not found: Already deleted");
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    // update LRU
    lru_update_lock(c, ce);

    pthread_mutex_unlock(&c->lock);
    return true;
}

bool tc_expire(Cache* c, command* str) {

    bool result = false;
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return result;
    }

    result = delete(c, str->key); 
    pthread_mutex_unlock(&c->lock);
    return result;
}

