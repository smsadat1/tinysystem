// cache implementations (hashmap)

#include "tinycache.h"

void init_cache(Cache* cache) {

    // any number of buckets
    cache->buckets = (cache_entry**)calloc(TC_BUCKET_COUNT, sizeof(cache_entry*));
    if (cache->buckets == NULL) {
       perror("Failed to allocate cache buckets");
       abort();
    }

    cache->entries = 0;
    cache->mem_used = 0;
    cache->mem_max = TC_MAX_MEMORY_MB * 1024ULL * 1024ULL;

    if (pthread_mutex_init(&cache->lock, NULL) != 0) {
        free(cache->buckets);
        cache->buckets = NULL;
        abort();
    }
}

// NULL whole cache before exit
void destroy_cache(Cache* cache) {
    free(cache->buckets);
    cache->mem_used = 0;
    cache->mem_max = 0;
}


uint64_t sdbm_hash(const char* key) {
    
    uint64_t hash = 0;
    int c;

    while ((c = *key++)) {
        // hash * 65599 + c
        hash = (uint64_t)c + (hash << 6) + (hash << 16) - hash; 
    }
    return hash;
}


bool insert(
    Cache* c, char* key, char* value, size_t value_len, int ttl
) {
    
    if (c == NULL || key == NULL ||
       (value == NULL && value_len > 0)) {
       return false;
    }

    size_t bucket_index = sdbm_hash(key) % TC_BUCKET_COUNT;
    // printf("bucket idx: %lu\n", bucket_index);
    
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
    new_cache_entry->value = NULL;
    if(value_len > 0) {
        new_cache_entry->value = malloc(value_len);

        if(new_cache_entry->value == NULL) {
            free(new_cache_entry->key);
            free(new_cache_entry);
            return false;
        }
        memcpy(new_cache_entry->value, value, value_len);
    }
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

    // increment entry & memory count
    c->entries += 1;   
    c->mem_used += new_cache_entry->value_len + strlen(new_cache_entry->key);

    // update LRU
    lru_insert_lock(c, new_cache_entry);

    pthread_mutex_unlock(&c->lock);

    return true;
}


cache_entry* search(Cache* c, char* key) {
    size_t bucket_index = sdbm_hash(key) % TC_BUCKET_COUNT;

    cache_entry* ce = c->buckets[bucket_index];
    if(ce == NULL) {
        return NULL;
    }

    while (ce != NULL)
    {
        if(strcmp(key, ce->key) == 0) return ce;
        ce = ce->next;
    }
    
    // should not reach here
    return NULL;
}

bool delete(Cache* c, char* key) {
    size_t bucket_index = sdbm_hash(key) % TC_BUCKET_COUNT;

    cache_entry* prev_entry = NULL;
    cache_entry* curr_entry = c->buckets[bucket_index];
    if(curr_entry == NULL) {
        return false;
    }

    while (curr_entry != NULL)
    {
        if(strcmp(curr_entry->key, key) == 0) {

            // head node in bucket
            if(prev_entry == NULL) {
                c->buckets[bucket_index] = curr_entry->next;
            }
            // last or middle node in bucket
            else {
                prev_entry->next = curr_entry->next;
            }

            // decrement count
            c->entries -= 1;
            c->mem_used -= (curr_entry->value_len + strlen(curr_entry->key));

            // update LRU (remove entry from LRU list before final deletion)
            lru_remove_lock(c, curr_entry);

            free(curr_entry->key);
            free(curr_entry->value);
            free(curr_entry);

            return true;
        }

        prev_entry = curr_entry;
        curr_entry = curr_entry->next;
    }
    
    return false;
}