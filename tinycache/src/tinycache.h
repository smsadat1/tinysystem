#ifndef TINYCACHE_H
#define TINYCACHE_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#define MAX 80 
#define TC_DEFAULT_MAX_MEM_MB 124
#define TC_MAX_ALLOWED_CONNECTION 80
#define TC_PORT 9736   // opposite of redis
#define TC_DEFAULT_TTL 300
#define TC_MAX_MEMORY_MB 128
#define TC_INTERVAL_SEC 5
#define TC_NO_EXPIRY 0
#define TC_BUCKET_COUNT INT16_MAX


typedef struct command_t {
    char *key;
    char *value;
    size_t value_len;
    int64_t ttl;
} command;

typedef enum {
    EVICTION_LRU,
    EVICTION_LFU,
} EvictionPolicy;

typedef struct cache_entry_t {
    
    char *key;
    void *value;
    size_t value_len;
    
    bool expiry;
    uint64_t expires_at;
    
    // count access frequency for LFU
    uint32_t frequency;    
    // Doubly linked LRU list
    struct cache_entry_t *lru_prev;
    struct cache_entry_t *lru_next;
    
    struct cache_entry_t *next;
} cache_entry;


typedef struct cache_t {
    
    cache_entry** buckets;
    size_t entries;
    size_t mem_used;
    size_t mem_max;
    
    EvictionPolicy policy;
    cache_entry* lru_head;
    cache_entry* lru_tail;
    
    pthread_mutex_t lock;
} Cache;

typedef struct WorkerConfig_t {
    int max_mem_mb;
    EvictionPolicy policy;
    Cache* cache;
} WorkerConfig;


void* worker(void* arg);
void tc_server(Cache* cache);

// logging
void tc_info(char* mesg);
void tc_error(char* mesg);


// command parsing from server
void parse_command(Cache* cache, int conn_fd);


// cache management api
bool tc_set(Cache* c, command* str);
void* tc_get(Cache* c, command* str);
bool tc_delete(Cache* c, command* str);
bool tc_exists(Cache* c, command* str);
bool tc_expire(Cache* c, command* str);
uint32_t tc_ttl(Cache* c, command* str);
void tc_stat(Cache* c);

// LFU
cache_entry* find_lfu_victim(Cache* c);

// LRU
void lru_insert_lock(Cache* c, cache_entry* ce);
void lru_update_lock(Cache* c, cache_entry* ce);
void lru_remove_lock(Cache* c, cache_entry* ce);


// core implementations over hashmap

void init_cache(Cache* cache);
void destroy_cache(Cache* cache);

// used by SET
bool insert(Cache* c, char* key, char* value, int value_len, int ttl);
// used by GET, TTL, EXPIRE, EXISTS
cache_entry* search(Cache* ce, char* key);
// used by DELETE
bool delete(Cache* c, char* key);


#endif // TINYCACHE_H