#include "container.h"

void set_hostname(struct utsname* uts, char* hostname) {
    
    // change hostname in UTS of child
    if(sethostname(hostname, strlen(hostname)) == -1)
        err(EXIT_FAILURE, "sethostname");

    // show hostname of child
    if (uname(uts) == -1)
        err(EXIT_FAILURE, "uname");

    printf("[INF] Hostname: %s\n", uts->nodename);    
}

void set_root_user() {
    
}