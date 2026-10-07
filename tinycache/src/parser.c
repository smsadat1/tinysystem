// command parser

#include "tinycache.h"

void parse(Cache* cache, char commandbuf[]) {

    command* c = (command*)malloc(sizeof(command));

    char* command_type = strtok(commandbuf, " ");

    if(strcmp(command_type, "SET") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .key_len = strlen(c.key),
            .value = strtok(NULL, ""),
            .value_len = strlen(c.value),
            .ttl = 300,
        };

        if(tc_set(cache, c)) 
            printf("OK\n");
        else 
            tc_error("Failed to set key");
    } 
    else if(strcmp(command_type, "GET") == 0) {

        command c = {
           .key = strtok(NULL, " "),
           .key_len = strlen(c.key),
           .value = "", .value_len = 0, .ttl = -1,
        };

        printf("Value: %s\n", tc_get(cache, c));
    } 
    else if(strcmp(command_type, "DELETE") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .key_len = strlen(c.key),
            .value = "", .value_len = 0, .ttl = -1,
        };

        if(tc_delete(cache, c)) 
            printf("OK\n");
        else 
            tc_error("Failed to delete key");
    } 
    else if(strcmp(command_type, "EXPIRE") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .key_len = strlen(c.key),
            .value = "", .value_len = 0, .ttl = -1,
        };

        if(tc_expire(cache, c)) 
            printf("OK\n");
        else 
            tc_error("Failed to expire key");
    }
    else if(strcmp(command_type, "EXIST") == 0) {
        
        command c = {
            .key = strtok(NULL, " "),
            .key_len = strlen(c.key),
            .value = "", .value_len = 0, .ttl = -1,
        };

        if(tc_exists(cache, c)) 
            printf("EXISTS\n");
        else 
            tc_error("DOES NOT EXIST");
    }
    else if(strcmp(command_type, "TTL") == 0) {
        
        command c = {
            .key = "", .key_len = 0,
            .value = "", .value_len = 0,
            .ttl = atoi(strtok(NULL, " ")),
        };

        if(tc_set(cache, c)) 
            printf("OK\n");
        else 
            tc_error("Failed to set TTL");
    }
    else if(strcmp(command_type, "PING") == 0 || strcmp(command_type, "PING\n") == 0 ) {
        puts("PONG");
    }
    else if(strcmp(command_type, "STAT") == 0) {
        
        printf("%s\n", tc_stat(cache));
    } 
    else {
        printf("Unknown command: %s\n", commandbuf);
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
            parse(cache, command_buff);
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