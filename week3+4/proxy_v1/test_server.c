/*
 * test_server.c -- Simple UDP server to test the proxy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "./utils/proxy_utils.h"

#define TEST_PORT 6000
#define MAXBUFSIZE 1000

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    unsigned char buf[MAXBUFSIZE];
    int bytes_received;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TEST_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("Test server listening on port %d...\n", TEST_PORT);

    // Receive messages in a loop
    while (1) {
        memset(buf, 0, MAXBUFSIZE);
        
        bytes_received = rec_proxy(sockfd, buf, MAXBUFSIZE-1, 0, 
                                  (struct sockaddr *)&client_addr, &client_len);
        
        if (bytes_received == -1) {
            perror("recvfrom");
            continue;
        }

        buf[bytes_received] = '\0';
        
        printf("Received %d bytes from %s:%d\n", 
               bytes_received,
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
        printf("Message: %s\n\n", buf);
    }

    close(sockfd);
    return 0;
}
