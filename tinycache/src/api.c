// cache api

#include "tinycache.h"


bool tc_set(cache_entry* ce, command str) {
    return true;
}

char* tc_get(cache_entry* ce, command str) {
    return "Hudai";
}

bool tc_delete(cache_entry* ce, command str) {
    return true;
}

bool tc_exists(cache_entry* ce, command str) {
    return true;
}

bool tc_expire(cache_entry* ce, command str) {
    return true;
}

bool tc_ttl(cache_entry* ce, command str) {
    return true;    
}

char* tc_stat(cache_entry* ce) {
    return "Hudai list";
}