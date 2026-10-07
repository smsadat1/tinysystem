// TCP server for receiving commands

#include "tinycache.h"

void tc_error(char* mesg) {
    printf("Error: %s\n", mesg);
    exit(0);
}

void tc_server(Cache* cache)
{
    int sock_fd, conn_fd;
    struct sockaddr_in server_addr = {0}, client = {0}; 

    printf("Initializing tinycache server...\n");

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        tc_error("Scoket creation failed");
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TC_PORT);


    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        tc_error("Socket bind failed...\n");
    } 

    if( listen(sock_fd, TC_MAX_ALLOWED_CONNECTION) == -1) {
        tc_error("Listen failed");
    }
    printf("Listening at port %u...\n", TC_PORT);
    
    socklen_t len = sizeof(client);
    if((conn_fd = accept(sock_fd, (struct sockaddr*)&client, &len)) == -1) {
        tc_error("Accept failed");
    }

    parse_command(cache, conn_fd);
    
    printf("Closing connection bye bye...\n");
    close(sock_fd);

    return 0;
}
