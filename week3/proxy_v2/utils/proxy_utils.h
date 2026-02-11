/*
 * proxy_utils.h -- helper utils for sending data in/out of the server while preserving sender intent
 */

#ifndef PROXY_UTILS_H
#define PROXY_UTILS_H

#include "../proxy_config.h"
#include "../../common.h"

// Standard socket/server libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/*
 * Proxy Usage:
 * 
 * The goal is to have all data pass through the proxy before reaching its destination. This allows us to track certain data such as
 * avg size of packets, latency, etc. The initial use for this is a latency simulator.
 * 
 * However, with UDP, destination address is not preserved, so the final address has to be included in the message buffer somehow.
 * We could track users on the proxy and utilize that to sort sender->destination, but when the server sends a message to a particular client,
 * the proxy has no way to know which client to send it to.
 * 
 * This adds a lot of friction, and the goal is to make it as easy as possible to integrate these tools into any program utilizing standard UDP sockets.
 * 
 * After a lot of thinking, I decided to create these intermediary files. These send and receive functions work virtually the same as standard sendto()
 * and recvfrom() functions, however, they add the destination's address and port to the packet itself. Thus, the caller does not have to change anything
 * about it's function signature (other than the function name itself), and the utility files manage it by themselves.
 * 
 * TLDR: These utilities prepend the destination address to the buffer and send that packet to the proxy, which handles it from there.
 */

 /*
  * prepend_address() -- given a packet and an address, populate the given buffer with the address+packet data
  */
int prepend_address(struct sockaddr *addr, unsigned char packet[MAXBUFSIZE], unsigned char buf[MAXBUFSIZE], size_t buflen);

/*
 * send_proxy() -- identical usage to sendto(), but prepends the intended address in the given packet, and sends through the proxy server
 */
int send_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, int flags, struct sockaddr *addr, socklen_t addr_len);

/*
 * rec_proxy() -- identical usage to recvfrom(), but extracts the prepended address from the packet and discards the proxy address
 */
int rec_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, int flags, struct sockaddr *addr, socklen_t *addr_len);

#endif