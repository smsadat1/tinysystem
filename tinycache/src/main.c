#include "tinycache.h"

int main(int argc, char* argv[])
{
    Cache cache;
    init_cache(&cache);
    
    WorkerConfig config = {
        .max_mem_mb = TC_DEFAULT_MAX_MEM_MB,
        .policy = EVICTION_LRU,
        .cache = &cache,
    };
    pthread_t worker_thread;

    for(int i = 1; i < argc; i++) {
        
        if (strcmp(argv[i], "--eviction") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing eviction policy\n");
                return EXIT_FAILURE;
            }
           
            const char *policy = argv[++i];
           
            if (strcmp(policy, "lru") == 0) {
                config.policy = EVICTION_LRU;
            } 
            else if (strcmp(policy, "lfu") == 0) {
                config.policy = EVICTION_LFU;
            } 
            else {
                fprintf(stderr, "Unknown eviction policy: %s\n", policy);
                return EXIT_FAILURE;
            }
        } 
        else if (strcmp(argv[i], "--mem_max_mb") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing memory limit\n");
                return EXIT_FAILURE;
            }

            config.max_mem_mb = atoi(argv[++i]);

            if (config.max_mem_mb <= 0) {
                fprintf(stderr, "Memory limit must be positive\n");
                return EXIT_FAILURE;
            }
        }
    }

    // launch async worker
    if(pthread_create(&worker_thread, NULL, worker, &config) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }
    pthread_detach(worker_thread);

    tc_server(&cache);

    tc_log(TC_DEBUG, "Destroying cache");
    destroy_cache(&cache);

    return 0;
}
