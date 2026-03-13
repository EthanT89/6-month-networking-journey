/*
 * test_client.c -- Simple UDP client that uses send_proxy to test the proxy
 */

#include "./utils/proxy_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TEST_SERVER_PORT 6000

int main() {
    int sockfd;
    struct sockaddr_in dest_addr;
    unsigned char message[MAXBUFSIZE];
    int bytes_sent;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Setup destination address (fake address for testing - what the server will eventually be)
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(TEST_SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

    printf("Test client started. Type messages to send through proxy.\n");
    printf("Destination: 127.0.0.1:%d\n", TEST_SERVER_PORT);
    printf("Proxy: 127.0.0.1:%d\n\n", PROXY_PORT_N);

    // Send messages in a loop
    while (1) {
        printf("Enter message (or 'quit' to exit): ");
        memset(message, 0, MAXBUFSIZE);
        fgets((char *)message, MAXBUFSIZE, stdin);
        
        // Remove newline
        size_t len = strlen((char *)message);
        if (len > 0 && message[len-1] == '\n') {
            message[len-1] = '\0';
            len--;
        }

        // Check for quit
        if (strcmp((char *)message, "quit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Use send_proxy instead of sendto
        bytes_sent = send_proxy(sockfd, message, len, 0, 
                               (struct sockaddr *)&dest_addr, 
                               sizeof(dest_addr));

        if (bytes_sent == -1) {
            perror("send_proxy");
        } else {
            printf("Sent %d bytes through proxy\n\n", bytes_sent);
        }
    }

    close(sockfd);
    return 0;
}
