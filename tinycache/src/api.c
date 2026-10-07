// cache api

#include "tinycache.h"


bool tc_set(Cache* c, command str) {
    
    bool result = false;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

char* tc_get(Cache* c, command str) {
    char* result;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

bool tc_delete(Cache* c, command str) {
    bool result = false;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

bool tc_exists(Cache* c, command str) {
    bool result = false;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

bool tc_expire(Cache* c, command str) {
    bool result = false;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

bool tc_ttl(Cache* c, command str) {
    bool result = false;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}

char* tc_stat(Cache* c) {
    char* result;
    pthread_mutex_lock(&c->lock);
    // do stuff
    pthread_mutex_unlock(&c->lock);
    return result;
}