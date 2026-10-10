// cache implementations (hashmap)

#include "tinycache.h"

void init_cache(Cache* cache) {

    // any number of buckets
    cache->buckets = (cache_entry**)malloc(sizeof(cache_entry*));
    cache->mem_used = 0;
    cache->mem_max = TC_MAX_MEMORY_MB * 1024 * 1024;

    pthread_mutex_init(&cache->lock, NULL);
}

// NULL whole cache before exit
void destroy_cache(Cache* cache) {
    free(cache->buckets);
    cache->mem_used = 0;
    cache->mem_max = 0;
}


uint8_t sdbm_hash(const char* key) {
    
    uint8_t hash = 0;
    int c;

    while ((c = *key++)) {
        // hash * 65599 + c
        hash = c + (hash << 6) + (hash << 16) - hash; 
    }
    return hash;
}


bool insert(
    Cache* c, char* key, char* value, int value_len, int ttl
) {
    
    int bucket_index = sdbm_hash(key) % TC_BUCKET_COUNT;
    
    cache_entry* new_cache_entry = (cache_entry*)malloc(sizeof(cache_entry));
    if(new_cache_entry == NULL) {
        return false;
    }

    // assign key
    new_cache_entry->key = strdup(key);
    if(new_cache_entry->key == NULL) {
        free(new_cache_entry);
        return false;
    }

    // assign value
    new_cache_entry->value = malloc(value_len);
    if(new_cache_entry->value == NULL) {
        free(new_cache_entry->key);
        free(new_cache_entry);
        return false;
    }
    memcpy(new_cache_entry->value, value, value_len);
    new_cache_entry->value_len = value_len;

    // assign expiry
    if(ttl == TC_NO_EXPIRY) {
        new_cache_entry->expiry = false;
        new_cache_entry->expires_at = 0;
    } 
    else {
        new_cache_entry->expires_at = time(NULL) + ttl;
        new_cache_entry->expiry = true;
    }

    // thread safety during entry
    pthread_mutex_lock(&c->lock);

    // add at the beginning of bucket
    new_cache_entry->next = c->buckets[bucket_index];
    c->buckets[bucket_index] = new_cache_entry;

    pthread_mutex_unlock(&c->lock);

    return true;
}


cache_entry* search(Cache* c, char* key) {
    size_t bucket_index = sdbm_hash(key) % TC_BUCKET_COUNT;

    cache_entry* ce = c->buckets[bucket_index];
    while (ce != NULL)
    {
        if(strcmp(key, ce->key) == 0) return ce;
        ce = ce->next;
    }
    
    return NULL;
}

bool delete(cache_entry* ce, char* key) {
    
    ce = NULL;
    key = "";
    return false;
}