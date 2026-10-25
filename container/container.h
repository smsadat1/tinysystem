#ifndef PROCMGR_H
#define PROCMGR_H

#define _GNU_SOURCE
#include <err.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>


#define TS_DEFAULT_HOSTNAME "tshost"
#define STACK_SIZE (1024*1024) // 1KB of stack for cloned child


void set_hostname(struct utsname* uts, char* hostname);

#endif // PROCMGR_H