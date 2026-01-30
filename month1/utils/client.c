/*
 * client.c -- A utility for creating and managing 'clients' connected to a server.
 *
 * To be used by the TCP Chat Server. Can be expanded to function for multiple servers.
 */

#include "./client.h"

void add_client(struct Client *client, struct LLClients *clients)
{
    client->next = NULL;

    if (clients->n != 0){
        client->next = clients->head;
        client->next->prev = client;
    } 
    
    client->prev = NULL;
    clients->head = client;
    clients->n++;
}

int remove_client(struct LLClients *clients, int sockfd)
{
    if (clients->n == 1){
        struct Client *p = clients->head;
        clients->head = NULL;
        clients->n = 0;
        free(p);
        return 1;
    }
    for (struct Client *p = clients->head; p != NULL; p = p->next){
        if (p->sockfd == sockfd){
            if (p->prev == NULL){
                p->next->prev = NULL;
                clients->head = p->next;
            } else if (p->next == NULL){
                p->prev->next = NULL;
            } else {
                p->prev->next = p->next;
                p->next->prev = p->prev;
            }

            clients->n--;
            free(p);
            return 1;
        }
    }
    return 0;
}

void print_clients(struct LLClients *clients)
{
    for (struct Client *p = clients->head; p != NULL; p = p->next){
        printf("Client %d:\nName: %s\n\n", p->sockfd, p->name);
    }
}

struct Client* create_client(int sockfd, unsigned char name[MAXCLIENTNAME])
{
    struct Client *client = calloc(1, sizeof *client);
    strcpy(client->name, name);
    client->sockfd = sockfd;

    return client;
}

struct Client* find_client_by_sockfd(int sockfd, struct LLClients *clients){
    for (struct Client *p = clients->head; p != NULL; p = p->next){
        if (p->sockfd == sockfd){
            return p;
        }
    }
    return NULL;
}

struct Client* find_client_by_name(unsigned char name[MAXCLIENTNAME], struct LLClients *clients){
    for (struct Client *p = clients->head; p != NULL; p = p->next){
        if (strcmp(name, p->name) == 0){
            return p;
        }
    }
    return NULL;
}