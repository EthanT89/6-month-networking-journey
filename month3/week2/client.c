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
#include "./utils/file_transfer.h"

/*
 * get_socket() -- create and return a TCP connection to the server's client port
 */
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

/*
 * identify_cmd_type() -- parse command string and return corresponding packet ID
 *
 * Valid commands: 'submit', 'status', 'results'
 */
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

/*
 * is_all_digits() -- validate that a string contains only numeric digits
 */
int is_all_digits(const char *str) {
    if (str == NULL || *str == '\0') return 0; // Handle null or empty string

    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0; // Found a non-digit character
        }
    }
    return 1; // All characters are digits
}

/*
 * validate_cmd_metadata() -- ensure metadata matches expected format for command type
 *
 * Submit commands accept any string, status/results require numeric job ID
 */
int validate_cmd_metadata(char *argv, int cmd_id){
    if (cmd_id == JOBSUBMITID){
        return 1;
    }

    return is_all_digits(argv);
}

int receive_results(int sockfd){
    unsigned char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);
    
    recv(sockfd, response, MAXBUFSIZE, 0);
    printf("%s\n", response);
    memset(response, 0, MAXBUFSIZE);
}

int send_packet(int sockfd, unsigned char *data, int offset){
    int bytes_sent = send(sockfd, data, offset, 0);
    if (bytes_sent == -1) {
        perror("Error: send packet\n");
        exit(1);
    }
    printf("%d bytes sent.\n", bytes_sent);
    return bytes_sent;
}

int validate_submission(int argc, char **argv){
    if (argc < 3){
        printf("usage: ./client [CMD] [METADATA]\n");
        exit(1);
    }

    int cmd_id = identify_cmd_type(argv[1]);
    if (cmd_id == -1){
        exit(1);
    }

    if (cmd_id == JOBSUBMITID){
        if (argc < 4){
            printf("usage: ./client submit [JOBTYPE] [FILEPATH]\n");
        }
    }

    int valid = validate_cmd_metadata(argv[2], cmd_id);
    if (valid == 0){
        printf("Job id must be a number.\n");
        exit(1);
    }

    return cmd_id;
}

void validate_file_path(char *path){
    FILE *fs = fopen(path, "r");

    if (fs == NULL){
        perror("Invalid file path\n");
        exit(1);
    }

    printf("valid file path\n");
}

int handle_send_id(int sockfd, unsigned char *metadata){
    int rv = send_packet(sockfd, metadata, strlen(metadata));
    if (rv > 0) return 1;
}

int handle_job_metadata(int sockfd, int job_type, unsigned char *metadata){
    if (job_type != JOBSUBMITID){
        printf("sending job id...\n");
        return handle_send_id(sockfd, metadata);
    }
    return send_file(sockfd, metadata);
}

int main(int argc, char **argv){
    int cmd_id = validate_submission(argc, argv);
    if (cmd_id == JOBSUBMITID) validate_file_path(argv[3]);

    printf("\nConnecting to server...\n");
    int sockfd = get_socket();
    int offset = 0;

    unsigned char job_header[MAXBUFSIZE];
    memset(job_header, 0, MAXBUFSIZE);

    // Send job header
    packi16(job_header+offset, APPID); offset += 2;
    packi16(job_header+offset, cmd_id); offset += 2;
    send_packet(sockfd, job_header, offset);

    // Send job specs
    if (cmd_id == JOBSUBMITID){
        offset = 0;
        
        memset(job_header, 0, MAXBUFSIZE);
        packi16(job_header+offset, strlen(argv[2])); offset += 2;
        strcpy(job_header+offset, argv[2]); offset += strlen(argv[2]);

        send_packet(sockfd, job_header, offset);
    }
        

    handle_job_metadata(sockfd, cmd_id, argv[3]);

    receive_results(sockfd);

    close(sockfd);
}