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
        
        char* key = strtok(NULL, " ");
        char* value = strtok(NULL, " ");

        command c = {
            .key = key,
            .value = value,
            .value_len = value ? strlen(value) : 0,
            .ttl = 300,
        };

        if (c.key == NULL || c.value == NULL) {
            send_result(conn_fd, "FAILED", 6);
        }

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

        int value_len = 0;
        void* value = tc_get(cache, &c, &value_len);
        if (value == NULL) {
            send_result(conn_fd, "(nil)", 5);
        } else {
            printf("Value length: %d\n", value_len);
            printf("Value bytes: ");
            fwrite(value, 1, value_len, stdout);
            printf("\n");

            send_result(conn_fd, value, value_len);
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

    // extra 1 bytes for newline delimiter
    char command_buff[TC_MAX_READ_BYTES+1] = {0};
    ssize_t n;

    while (true)
    {
        n = read(conn_fd, command_buff, TC_MAX_READ_BYTES);
        
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