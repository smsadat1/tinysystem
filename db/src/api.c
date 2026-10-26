// cache api

#include "tinycache.h"


bool tc_set(Cache* c, command* str) {
    
    if (c == NULL || str == NULL ||
        str->key == NULL ||
        (str->value == NULL && str->value_len > 0)) {
        return false;
    }
    
    return insert(
        c,
        str->key,
        str->value, str->value_len,
        TC_NO_EXPIRY
    );
}

void* tc_get(Cache* c, command* str, size_t* value_len) {
    
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
        *value_len = ce->value_len;
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
        tc_log(TC_DEBUG, "Key not found: Already deleted");
        pthread_mutex_unlock(&c->lock);
        return false;
    }

    // If there's expiry then check remainin time to decide
    if(ce->expiry && ce->expires_at <= (uint64_t)time(NULL)) {
        
        tc_log(TC_DEBUG, "Key found: Attempting lazy deletion");
        
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
        ce->expires_at = (uint64_t)time(NULL) + str->ttl;
        ce->expiry = true;
        pthread_mutex_unlock(&c->lock);
        return true;
    }

    // set new expiry
    if (ce->expiry) {
        
        // check if already expired
        if(ce->expires_at < (uint64_t)time(NULL)) {
            pthread_mutex_unlock(&c->lock);
            return false;
        }
        
        // increment over previous expiry
        ce->expires_at += str->ttl;
        pthread_mutex_unlock(&c->lock);
        return true;
    }

    // should not reach here
    return false;
}

int32_t tc_ttl(Cache* c, command* str) {
    pthread_mutex_lock(&c->lock);

    cache_entry* ce = search(c, str->key);
    if(ce == NULL) {
        pthread_mutex_unlock(&c->lock);
        return TC_TTL_NOT_EXIST;
    }

    if(ce->expires_at != 0 &&
    ce->expires_at <= (uint64_t)time(NULL)) {
        // lazy deletion
        delete(c, str->key);
        pthread_mutex_unlock(&c->lock);
        return TC_TTL_EXPIRED;
    }

    if (ce->expires_at == 0) {
       pthread_mutex_unlock(&c->lock);
       return TC_TTL_EXPIRED;
    }

    int32_t remaining = (int32_t)(ce->expires_at - (uint64_t)time(NULL));
    pthread_mutex_unlock(&c->lock);
    return remaining;
}

void tc_stat(int conn_fd, Cache* c) {
    pthread_mutex_lock(&c->lock);

    char newline = '\n';
    char forward_slash = '/';

    // Total entries: c->entries
    char total_entires[16] = "Total entries: "; 
    send_result(conn_fd, &total_entires, strlen(total_entires));
    send_result(conn_fd, &c->entries, sizeof(c->entries));
    send_result(conn_fd, &newline, 1);

    // Memory used (bytes): c->mem_used/c->mem_max
    char memory_used[22] = "Memory used (bytes): ";
    send_result(conn_fd, &memory_used, strlen(memory_used));
    send_result(conn_fd, &c->mem_used, sizeof(c->mem_used));
    send_result(conn_fd, &forward_slash, 1);
    send_result(conn_fd, &c->mem_max, sizeof(c->mem_max));
    send_result(conn_fd, &newline, 1);

    if(c->policy == EVICTION_LRU) {
        char policy_lru[22] =  "Eviction policy: LRU\n";
        send_result(conn_fd, &policy_lru, strlen(policy_lru));
    } 
    else if (c->policy == EVICTION_LFU)
    {
        char policy_lfu[22] =  "Eviction policy: LRU\n";
        send_result(conn_fd, &policy_lfu, strlen(policy_lfu));
    }

    pthread_mutex_unlock(&c->lock);
}