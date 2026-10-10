#include "tinycache.h"

int main()
{
    Cache cache;
    init_cache(&cache);

    tc_server(&cache);
    
    destroy_cache(&cache);

    return 0;
}
