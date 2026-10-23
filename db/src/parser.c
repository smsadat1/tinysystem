// command parser

#include "tinycache.h"

void parse(int conn_fd, Cache* cache, char commandbuf[]) {

    // replace newline with null terminator
    char* newline = strchr(commandbuf, '\n');
    if(newline) {
        *newline = '\0';
    }

    char* command_type = strtok(commandbuf, " ");

    if(strcmp(command_type, "SET") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .value = strtok(NULL, ""),
            .ttl = 300,
        };
        c.value_len = strlen(c.value);

        if(tc_set(cache, &c)) 
            send_result(conn_fd, "OK", 2);
        else 
            send_result(conn_fd, "FAILED", 6);
    } 
    else if(strcmp(command_type, "GET") == 0) {

        command c = {
           .key = strtok(NULL, " "),
           .value = "", .value_len = 0, .ttl = -1,
        };

        char* value = tc_get(cache, &c);
        if (value == NULL) {
            send_result(conn_fd, "(nil)", 5);
        } else {
            send_result(conn_fd, value, sizeof(value));
            free(value);
        }
    } 
    else if(strcmp(command_type, "DELETE") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .value = "", .value_len = 0, 
            .ttl = -1,  // no worries, delete() ignores ttl
        };

        if(tc_delete(cache, &c)) 
            send_result(conn_fd, "OK", 2);
        else 
            send_result(conn_fd, "FAILED", 6);
    } 
    else if(strcmp(command_type, "EXPIRE") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .value = "", .value_len = 0, 
            .ttl = atoi(strtok(NULL, " ")),
        };

        if(tc_expire(cache, &c)) 
            send_result(conn_fd, "OK", 2);
        else 
            send_result(conn_fd, "FAILED", 6);
    }
    else if(strcmp(command_type, "EXIST") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .value = "", .value_len = 0, 
            .ttl = -1,
        };

        if(tc_exists(cache, &c)) 
            send_result(conn_fd, "EXISTS", 6);
        else 
            send_result(conn_fd, "EXISTS", 14);
    }
    else if(strcmp(command_type, "TTL") == 0) {
        
        command c = {
            .key = strtok(NULL, " "), 
            .value = "", .value_len = 0,
            .ttl = -1,
        };

        uint32_t ttl_amount = tc_ttl(cache, &c);
        send_result(conn_fd, ttl_amount, sizeof(ttl_amount));
    }
    else if(strcmp(command_type, "PING") == 0 || strcmp(command_type, "PING\n") == 0 ) {
        send_result(conn_fd, "PONG", 4);
    }
    else if(strcmp(command_type, "STAT") == 0 || strcmp(command_type, "STAT\n") == 0 ) {
        tc_stat(conn_fd, cache);
    } 
    else {
        // unknown command
        send_result(conn_fd, "", 0);
    }
}

void parse_command(Cache* cache, int conn_fd) {

    char command_buff[MAX] = {0};
    int n;

    while (true)
    {
        n = read(conn_fd, command_buff, sizeof(command_buff));
        
        if(n > 0) {
            command_buff[n] = '\0'; // null terminate each command
            printf("%s", command_buff);
            parse(conn_fd, cache, command_buff);
        }
        else if (n == 0)
        {
            return;
        } 
        else 
        {
            perror("read");
        }
    }
}