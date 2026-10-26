// TCP server for receiving commands

#include "tinycache.h"

void tc_error(char* mesg) {
    printf("Error: %s\n", mesg);
    exit(0);
}

void tc_info(char* mesg) {
    printf("Error: %s\n", mesg);
}

int tc_server(Cache* cache)
{
    int sock_fd, conn_fd;
    struct sockaddr_in server_addr = {0}, client = {0}; 

    tc_log(TC_INFO, "Initializing tinycache server...");

    tc_log(TC_DEBUG, "Creating socket");
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        tc_log(TC_FATAL, "Socket creation failed");
        close(sock_fd);
        return TC_SERVER_ERROR;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TC_PORT);

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    tc_log(TC_DEBUG, "Binding socket");
    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        tc_log(TC_FATAL, "Socket bind failed");
        close(sock_fd);
        return TC_SERVER_ERROR;
    } 

    tc_log(TC_DEBUG, "Start listening socket");
    if( listen(sock_fd, TC_MAX_ALLOWED_CONNECTION) == -1) {
        tc_log(TC_FATAL, "Socket listen failed");
        close(sock_fd);
        return TC_SERVER_ERROR;
    }
    tc_log(TC_INFO, "Listening at port: %d...", TC_PORT);

    socklen_t len = sizeof(client);
    if((conn_fd = accept(sock_fd, (struct sockaddr*)&client, &len)) == -1) {
        tc_log(TC_ERROR, "Accept failed");
        close(sock_fd);
        return TC_SERVER_ERROR;
    }

    parse_command(cache, conn_fd);

    tc_log(TC_INFO, "Closing connection bye bye...");
    close(sock_fd);
    
    return 0;
}
