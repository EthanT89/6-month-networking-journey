/*
 * client.h -- header file for client.c, client management utilities
 */

#ifndef _CLIENT_H
#define _CLIENT_H

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../common.h"
#include <netinet/in.h>

/*
 * Client -- represents a client connected to the chat server
 */
struct Client {
    int sockfd;
    unsigned char name[MAXCLIENTNAME];
    struct sockaddr_in address;
    struct Client *next;
    struct Client *prev;
};

/*
 * LLClients -- A linked list of connected clients using the Client struct
 */
struct LLClients {
    struct Client *head;
    int n; // num of connected clients
};

/*
 * add_client() -- handles adding a new client to an existing client list
 * The client struct must be made before calling this
 */
void add_client(struct Client *client, struct LLClients *clients);

/*
 * remove_client() -- handles removing a client from an existing 
 * client list using the client's socket file descriptor
 */
int remove_client(struct LLClients *clients, int sockfd);

/*
 * print_clients() -- loops through a client list and prints their socket file descriptor and name
 */
void print_clients(struct LLClients *clients);

/*
 * create_client() -- given a socket file descriptor and name, returns a pointer to a new client struct
 */
struct Client* create_client(int sockfd, unsigned char name[MAXCLIENTNAME]);


/*
 * find_client_by_sockfd() -- given a socket file descriptor, returns a pointer to the located client if found
 */
struct Client* find_client_by_sockfd(int sockfd, struct LLClients *clients);

/*
 * find_client_by_name() -- given a client name, returns a pointer to the located client if found
 */
struct Client* find_client_by_name(unsigned char name[MAXCLIENTNAME], struct LLClients *clients);

#endif