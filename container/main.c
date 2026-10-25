#include "container.h"

int spawn_isolated_process(void* args) {
    struct utsname uts;
    char ns[128];
    pid_t pid;

    set_hostname(&uts, TS_DEFAULT_HOSTNAME);

    // show child's PID in it's own view
    pid = getpid();
    printf("[INF] PID: %d\n", pid);
    

    sleep(2);
    return 0;
}

int main(int argc, char const *argv[])
{
    char *stack;         // start of stack buffer
    char *stack_top;      // end of stack buffer 
    pid_t child_pid;
    struct utsname uts;

    if (argc < 2) {
       fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
       exit(EXIT_SUCCESS);
    }

    stack = mmap(
        NULL, 
        STACK_SIZE, 
        PROT_READ | PROT_WRITE | PROT_EXEC, 
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK | MAP_GROWSDOWN, 
        -1,
        0
    );

    if(stack == MAP_FAILED)
        err(EXIT_FAILURE, "clone");

    // stack grows downwards
    stack_top = stack + STACK_SIZE;

    // spawn process with clone()
    child_pid = clone(
        spawn_isolated_process, 
        stack_top, 
        CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWCGROUP |
        SIGCHLD, 
        (void*)argv[1]
    );

    if(child_pid == -1)
        err(EXIT_FAILURE, "clone");

    // give child some time before exit
    sleep(1);

    if(uname(&uts) == -1) 
        err(EXIT_FAILURE, "uname");

    if(waitpid(child_pid, NULL, 0) == -1) 
        err(EXIT_FAILURE, "waitpid");

    puts("[INF] Process terminated");

    exit(EXIT_SUCCESS);
}
