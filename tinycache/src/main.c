#include "tinycache.h"

int main(int argc, char const *argv[])
{
    Cache cache;
    init_cache(&cache);

    tc_server(&cache);
    
    destroy_cache(&cache);

    return 0;
}
