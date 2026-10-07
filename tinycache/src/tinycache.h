#ifndef TINYCACHE_H
#define TINYCACHE_H

#include <stdbool.h>
#include <stdio.h> 
#include <string.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#define MAX 80 
#define TC_MAX_ALLOWED_CONNECTION 80
#define TC_PORT 9736   // opposite of redis
#define TC_DEFAULT_TTL 300
#define TC_MAX_MEMORY_MB 128
#define TC_LRU_INTERVAL_SEC 5


typedef struct command_t {
    char *key;
    size_t key_len;
    char *value;
    size_t value_len;
    int64_t ttl;
} command;

typedef struct cache_entry_t {
    char *key;
    void *value;
    size_t value_len;

    uint64_t expires_at;

    struct cache_entry *prev;
    struct cache_entry *next;
} cache_entry;


typedef struct cache_t {
    cache_entry* ce;
    size_t mem_used;
    size_t mem_max;
    pthread_mutex_t lock;
} Cache;


void init_cache(Cache* cache);
void destroy_cache(Cache* cache);

void tc_server(Cache* cache);

void tc_error(char* mesg);

void parse_command(Cache* cache, int conn_fd);

bool tc_set(cache_entry* ce, command str);
char* tc_get(cache_entry* ce, command str);
bool tc_delete(cache_entry* ce, command str);
bool tc_exists(cache_entry* ce, command str);
bool tc_expire(cache_entry* ce, command str);
bool tc_ttl(cache_entry* ce, command str);
char* tc_stat(cache_entry* ce);


#endif // TINYCACHE_H