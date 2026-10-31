#include "tinycache.h"

/*
*   Persistence strategy:
*   Hash partitioned append only binary records
*   Murmurhash3 well known for fair distribution is used to avoid hot partition
*   Each partitions are separate binary files under TINYCACHE_LOCATION/record/ 
*/

bool tc_add_record(char *key, void *value, size_t value_len) {

    // Select the record partition to append using hash
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;

    char* record_file;
    sprintf(record_file, "/srv/archive/record/%d.bin", partition);
    
    FILE* record_fp = fopen(record_file, O_APPEND);
    if (record_fp == NULL)
        return false;

/*

    On disk record format for SET (optimized for reverse scanning)
    |---------------------------------------------------------------------------------------------|
    | value_len bytes | 4 bytes   | key_len bytes | 4 bytes  | 1 byte  | 4 bytes | 4 bytes        |
    |-----------------|-----------|---------------|----------|---------|---------|----------------|
    | VALUE           | value_len | key           | key_len  | op      | CRC32C  | MAGIC (4bytes) |
    |-----------------|-----------|---------------|----------|---------|---------|----------------|

*/

    size_t key_len = strlen(key);
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef*)value, value_len);

    fwrite(value, 1, value_len, record_fp);
    fwrite(value_len, 1, 4, record_fp);
    fwrite(key, 1, key_len, record_fp);
    fwrite(key_len, 1, 4, record_fp);
    fwrite(OP_SET, 1, 1, record_fp);
    fwrite(crc, 1, 1, record_file);
    fwrite(TC_RECORD_MAGIC, 1, 4, record_file);
    
    // Make sure disk write
    fflush(record_fp);
    fsync(fileno(record_fp));
    fclose(record_fp);
}

void* tc_get_record(char* key) {

    // 1. Get hash of key
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;

    // 2. Select the partition to search
    char* record_file;
    sprintf(record_file, "/srv/archive/record/%d.bin", partition);

    FILE* record_fp = fopen(record_file, O_RDONLY);
    if (record_fp == NULL)
        return false;

    // 3. Reverse scan to find the latest mutation earliest
}

bool tc_delete_record(char* key) {

    // Select the record partition to append using hash
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;

    char* record_file;
    sprintf(record_file, "/srv/archive/record/%d.bin", partition);
    
    FILE* record_fp = fopen(record_file, O_APPEND);
    if (record_fp == NULL)
        return false;

/*

    On disk record format for DELETE (optimized for reverse scanning)
    |-----------------------------------------------------|
    | key_len bytes | 4 bytes  | 1 byte  | 4 bytes        |
    |---------------|----------|---------|----------------|
    | key           | key_len  | op      | MAGIC (4bytes) |
    |---------------|----------|---------|----------------|

*/

    size_t key_len = strlen(key);

    fwrite(key_len, 1, 4, record_fp);
    fwrite(key, 1, strlen(key), record_fp);
    fwrite(OP_DELETE, 1, 1, record_fp);
    fwrite(TC_RECORD_MAGIC, 1, 4, record_file);
    
    // Make sure disk write
    fflush(record_fp);
    fsync(fileno(record_fp));
    fclose(record_fp);
}