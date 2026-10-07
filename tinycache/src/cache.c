// cache implementations (hashmap)

#include "tinycache.h"

void init_cache(Cache* cache) {
    cache = (Cache*)malloc(sizeof(Cache));
    cache->ce = (cache_entry*)malloc(sizeof(cache_entry));
    cache->mem_used = 0;
    cache->mem_max = TC_MAX_MEMORY_MB;
}

void destroy_cache(Cache* cache) {
    free(cache);
}