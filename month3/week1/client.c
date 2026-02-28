/*
 * client.c -- client request management and facilitation
 */

// Main imports
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// custom imports
#include "./common.h"
#include "./utils/buffer_manipulation.h"

int get_socket(){
    int sockfd, rv;
    int yes = 1;
    struct addrinfo *res, *p, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

        // first, use getaddrinfo() to find some addresses!
    if ((rv = getaddrinfo(NULL, CLIENT_PORT, &hints, &res)) != 0){
        perror("server: getaddrinfo\n");
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next){
        // first, try to make a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket\n");
            continue;
        }

        // no more "address already in use" error :)
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        printf("Successfully connected to server!\n\n");
        break;
    }

    if (p == NULL){
        perror("server: start\n");
        exit(EXIT_FAILURE);
    }

    if ((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
        perror("client: connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int identify_cmd_type(char *argv){
    if (strlen(argv) == 6 && strncmp(argv, "submit", 6) == 0){
        return JOBSUBMITID;
    }
    if (strlen(argv) == 6 && strncmp(argv, "status", 6) == 0){
        return JOBSTATUSID;
    }
    if (strlen(argv) == 7 && strncmp(argv, "results", 7) == 0){
        return JOBRESULTID;
    }

    printf("job types: 'submit', 'status', 'results'\n");
    return -1;
}

int is_all_digits(const char *str) {
    if (str == NULL || *str == '\0') return 0; // Handle null or empty string

    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0; // Found a non-digit character
        }
    }
    return 1; // All characters are digits
}

int validate_cmd_metadata(char *argv, int cmd_id){
    if (cmd_id == JOBSUBMITID){
        return 1;
    }

    return is_all_digits(argv);
}

int main(int argc, char **argv){
    if (argc < 3){
        printf("usage: ./client [CMD] [METADATA]\n");
        exit(1);
    }

    printf("\nConnecting to server...\n");
    int sockfd = get_socket();
    unsigned char metadata[MAXJOBMETADATASIZE];
    unsigned char full_cmd[MAXBUFSIZE];
    unsigned char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);
    memset(full_cmd, 0, MAXBUFSIZE);
    memset(metadata, 0, MAXJOBMETADATASIZE);
    int offset = 0;

    int cmd_id = identify_cmd_type(argv[1]);
    int valid = validate_cmd_metadata(argv[2], cmd_id);

    if (cmd_id == -1){
        close(sockfd);
        exit(1);
    }
    if (valid == 0){
        printf("Job id must be a number.\n");
        close(sockfd);
        exit(1);
    }

    memset(metadata, 0, MAXJOBMETADATASIZE);
    strncpy(metadata, argv[2], MAXJOBMETADATASIZE);

    packi16(full_cmd+offset, APPID); offset += 2;
    packi16(full_cmd+offset, cmd_id); offset += 2;
    memcpy(full_cmd+offset, metadata, MAXJOBMETADATASIZE); offset += strlen(metadata);

    int bytes_sent = send(sockfd, full_cmd, offset, 0);
    printf("\n%d bytes sent.\n\n", bytes_sent);

    recv(sockfd, response, MAXBUFSIZE, 0);
    printf("%s\n", response);
    memset(response, 0, MAXBUFSIZE);

    close(sockfd);
}