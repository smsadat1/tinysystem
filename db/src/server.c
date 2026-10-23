// TCP server for receiving commands

#include "tinycache.h"

void tc_error(char* mesg) {
    printf("Error: %s\n", mesg);
    exit(0);
}

void tc_info(char* mesg) {
    printf("Error: %s\n", mesg);
}

void tc_server(Cache* cache)
{
    int sock_fd, conn_fd;
    struct sockaddr_in server_addr = {0}, client = {0}; 

    tc_log("Initializing tinycache server...", TC_INFO);

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        tc_log("Socket creation failed", TC_FATAL);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)atoi(TC_PORT));


    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        tc_log("Socket bind failed", TC_FATAL);
    } 

    if( listen(sock_fd, TC_MAX_ALLOWED_CONNECTION) == -1) {
        tc_log("Socket listen failed", TC_FATAL);
    }
    char* portlog = strcat("Listening at port ", TC_PORT);
    tc_log(portlog, TC_INFO);
    
    socklen_t len = sizeof(client);
    if((conn_fd = accept(sock_fd, (struct sockaddr*)&client, &len)) == -1) {
        tc_log("Accept failed", TC_ERROR);
    }

    parse_command(cache, conn_fd);

    tc_log("Closing connection bye bye...", TC_INFO);
    close(sock_fd);
}
