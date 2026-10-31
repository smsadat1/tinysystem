#include "tinycache.h"

// Persistent operations on disk

void* reverse_scan(int record_fd, const char* target_key, size_t key_len) {
    struct stat st;

    if(fstat(record_fd, &st) == -1)
        return NULL;

    off_t offset = st.st_size;
    char* line = NULL;
    size_t len = 0;
    size_t cap = 0;
    char block[TC_RECORD_REVSCAN_BLOCK_SIZE];

    while (offset > 0)
    {
        /* code */
    }
    
    // key not found
    goto fail;

fail:
    free(line);
    return NULL;
}