// TCP server for receiving commands

#include "tinycache.h"

void tc_log(char* mesg, TC_LOG_LEVEL level) {
    
    static char timestr[200];
    static time_t t;
    static struct tm* tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if(tmp == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    if (strftime(timestr, sizeof(timestr), "%d/%m/%Y %H:%M:%S", tmp) == 0) {
        fprintf(stderr, "strftime returned 0");
        exit(EXIT_FAILURE);
    }

    switch (level)
    {
    case TC_INFO:
        printf("[INF][%s%s%s] %s\n", 
            TC_LOG_GREY, timestr, TC_LOG_COLOR_TERMINATE,
            mesg
        );    
        break;

    case TC_WARNING:
        printf("[%sWRN%s][%s%s%s] %s%s%s\n", 
            TC_LOG_YELLOW, TC_LOG_COLOR_TERMINATE, 
            TC_LOG_GREY, timestr, TC_LOG_COLOR_TERMINATE, 
            TC_LOG_YELLOW, mesg, TC_LOG_COLOR_TERMINATE
        );
        break;

    case TC_ERROR:
        printf("[%sERR%s][%s%s%s] %s%s%s\n", 
            TC_LOG_RED, TC_LOG_COLOR_TERMINATE, 
            TC_LOG_GREY, timestr, TC_LOG_COLOR_TERMINATE, 
            TC_LOG_RED, mesg, TC_LOG_COLOR_TERMINATE
        );
        break;

    case TC_FATAL:
        printf("[%sFTL%s][%s%s%s] %s%s%s\n", 
            TC_LOG_DEEP_RED, TC_LOG_COLOR_TERMINATE,
            TC_LOG_GREY, timestr, TC_LOG_COLOR_TERMINATE, 
            TC_LOG_DEEP_RED, mesg, TC_LOG_COLOR_TERMINATE
        );
        exit(-1);    

    default:
        break;
    }
}
