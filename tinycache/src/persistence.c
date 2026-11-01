#include "tinycache.h"

/*
*   Persistence strategy:
*   Hash partitioned append only binary records
*   Each partitions are separate binary files under TINYCACHE_LOCATION/record/ 
*   Reverse scanning is used for fetching values from associated key
*   Because last mutation is all what matters for querying value from key
*/

bool tc_add_record(char *key, void *value, size_t value_len) {

    // Select the record partition to append using hash
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;

    struct stat st;
    char* record_file;
    sprintf(record_file, "/srv/archive/record/%llu.bin", partition);

/*

    On disk record format for SET (optimized for reverse scanning)
    |-----------------------------------------------------------------------------------|
    | value_len bytes | 4 bytes   | key_len bytes | 2 bytes  | 1 byte  | 4 bytes        |
    |-----------------|-----------|---------------|----------|---------|----------------|
    | VALUE           | value_len | key           | key_len  | op      | MAGIC (4bytes) |
    |-----------------|-----------|---------------|----------|---------|----------------|

*/

    int record_fd = open(record_file, O_CREAT | O_WRONLY, 0644);

    if(fstat(record_fd, &st) == -1) {
        perror("fstat");
        return false;
    }

    off_t record_offset = st.st_size;    
    size_t keylen = strlen(key);
    uint8_t op = RECORD_OP_DELETE;
    uint32_t magic = TC_RECORD_MAGIC;

    pwrite_all(record_fd, value, value_len, record_offset);
    record_offset += (off_t)value_len;

    pwrite_all(record_fd, &key, keylen, record_offset);
    record_offset += (off_t)keylen;

    pwrite_all(record_fd, &keylen, sizeof(keylen), record_offset);
    record_offset += sizeof(keylen);

    pwrite_all(record_fd, &op, sizeof(op), record_offset);
    record_offset += sizeof(op);

    pwrite_all(record_fd, &magic, sizeof(magic), record_offset);

    
    // disk synchronization for persistence
    if(fdatasync(record_fd) == -1) {
        perror("fdatasync");
        return false;
    }

    close(record_fd);
    return true;
}

// TODO: change it to 
//  bool tc_get_record(char* key, void** out_value, size_t* valuelen)
void* tc_get_record(char* key) {

    // Select the record partition to append using hash
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;
    
    struct stat st;
    char* record_file;
    sprintf(record_file, "/srv/archive/record/%llu.bin", partition);

    int record_fd = open(record_file, O_CREAT | O_WRONLY, 0644);

    if(fstat(record_fd, &st) == -1) {
        perror("fstat");
        return false;
    }

    off_t record_offset = st.st_size;    

    void **value;
    size_t *valuelen;

    // TODO: handle it's return values
    revscan(record_fd, key, strlen(key), record_offset, value, valuelen);
}

bool tc_delete_record(char* key) {

    // Select the record partition to append using hash
    uint64_t partition = sdbm_hash(key) % TC_MAX_RECORD_PARTITION;

    struct stat st;
    char* record_file;
    sprintf(record_file, "/srv/archive/record/%llu.bin", partition);
    
/*

    On disk record format for DELETE (optimized for reverse scanning)
    |-----------------------------------------------------|
    | key_len bytes | 2 bytes  | 1 byte  | 4 bytes        |
    |---------------|----------|---------|----------------|
    | key           | key_len  | op      | MAGIC (4bytes) |
    |---------------|----------|---------|----------------|

*/


    int record_fd = open(record_file, O_CREAT | O_WRONLY, 0644);

    if(fstat(record_fd, &st) == -1) {
        perror("fstat");
        return false;
    }

    off_t record_offset = st.st_size;    
    size_t keylen = strlen(key);
    uint8_t op = RECORD_OP_SET;
    uint32_t magic = TC_RECORD_MAGIC;

    pwrite_all(record_fd, &key, keylen, record_offset);
    record_offset += (off_t)keylen;

    pwrite_all(record_fd, &keylen, sizeof(keylen), record_offset);
    record_offset += sizeof(keylen);

    pwrite_all(record_fd, &op, sizeof(op), record_offset);
    record_offset += sizeof(op);

    pwrite_all(record_fd, &magic, sizeof(magic), record_offset);

    // disk synchronization for persistence
    if(fdatasync(record_fd) == -1) {
        perror("fdatasync");
        return false;
    }

    close(record_fd);
    return true;
}