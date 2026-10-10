#include "tinycache.h"

/*
    set policies during startup 
    
    Defaults:
    eviction=lru
    max_mem=128mb

    Command 

    POLICY eviction [LRU or LFU] max_mem [MB size]
*/
void set_policy(char* policy) {

}