/*
 * chat_server.c -- An echo-based chat server that handles multiple clients using poll()
 */

#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "./common.h"
#include "./utils/pfds.h"
#include "./utils/client.h"
#include <netinet/in.h>
#include <unistd.h>

#define MAXBACKLOG 10

/*
 * get_listener() -- create and return a listener socket. Socket is set to listening and accept()'ing
 * must be done separately.
 */
int get_listener()
{
    int sockfd, rv;
    int yes = 1;
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // First, use getaddrinfo() to find a bindable port
    // int getaddrinfo(const char *hostname, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &res)) != 0) {
        perror("server: setup");
        exit(1);
    }

    // Loop through the results and bind with the first viable socket
    for (p = res; p != NULL; p = p->ai_next){
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            fprintf(stderr, "server: socket\n");
            continue;
        }

        // no more "address already in use" error :)
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) != 0){
            fprintf(stderr, "server: bind\n");
            continue;
        }

        break;
    }

    if (p == NULL){
        perror("server: setup\n");
        exit(-1);
    }

    if (listen(sockfd, MAXBACKLOG) == -1){
        perror("server: listen\n");
        exit(1);
    }

    return sockfd;
}

/* 
 * get_newfd() -- given a listener socket and client list, accept the next client in queue and return the 
 * new socket file descriptor, then add the new client to the client list.
 */
int get_newfd(int sockfd, struct LLClients *clients)
{
    int newfd;
    struct sockaddr_in remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;
    unsigned char name[MAXCLIENTNAME];
    memset(name, 0, MAXCLIENTNAME);

    if ((newfd = accept(sockfd, (struct sockaddr *)&remoteaddr, &addrlen)) == -1){
        perror("server: accept\n");
        return -1;
    }
    printf("New connection from port %d - %d\n", remoteaddr.sin_port, newfd);

    send(newfd, "Welcome! Happy to have you here.\n", 34, 0);
    send(newfd, "Please enter a username (max 10 characters):\n", 46, 0);

    struct Client *client = create_client(newfd, NAMENOTSET);
    add_client(client, clients);

    return newfd;
}

/*
 * is_direct_message() -- given a received message (buf), determine if the message is intended to be a
 * direct message by checking if the message starts with "@". Returns the number of characters following the
 * "@" (the username). Returns 0 if not a direct message.
 */
int is_direct_message(unsigned char buf[MAXBUFSIZE]){
    size_t len = strlen(buf);
    
    if (len <= 0){
        return 0;
    }

    if (buf[0] == '@'){
        int name_len = 0;

        // find how many chars the tagged name is
        for (int i = 1; i < len; i++){
            if (buf[i] != ' '){
                name_len++;
                continue;
            }
            return name_len;
        }
        return 0;
    }
    return 0;
}

/*
 * is_command() -- given a received message (buf), determine if the message is intended to be a command call
 * by checking if the message starts with "/". Returns the number of characters following the "/" (command name)
 * if it is a command, otherwise 0.
 */
int is_command(unsigned char buf[MAXBUFSIZE]){
    size_t len = strlen(buf);
    
    if (len <= 0){
        return 0;
    }

    if (buf[0] == '/'){
        int cmd_len = 0;

        // find how many chars the command is
        for (int i = 1; i < len; i++){
            if (buf[i] != ' ' && buf[i] != '\r' && buf[i] != '\n'){
                cmd_len++;
                continue;
            }
            return cmd_len;
        }
        return cmd_len;
    }
    return 0;
}

/*
 * build_message() -- construct the final outgoing message constructed as "username: message".
 */
void build_message(struct Client *client, unsigned char msg[MAXBUFSIZE], unsigned char fullmsg[MAXBROADCAST]){
    unsigned char name[MAXCLIENTNAME];
    strcpy(name, client->name);
    strcpy(fullmsg, name);
    strcpy(fullmsg+strlen(name), ": ");
    strcpy(fullmsg+strlen(name)+2, msg);
}

/* 
 * broadcast() -- send a message to every client currently connected to the server.
 */
void broadcast(struct pollfd **pfds, int fd_count, unsigned char msg[MAXBROADCAST], int j)
{
    for (int i = 1; i < fd_count; i++){
        if (i == j) {continue; }

        int sockfd = (*pfds)[i].fd;
        if(send(sockfd, msg, strlen(msg), 0) == -1){
            perror("server: broadcast");
        }
    }
}

/*
 * handle_cmd() -- if a message is determine to be a command, handle the command logic here.
 *
 * /help -- lists all available commands
 * /users -- lists all actively connected users
 * /myname -- prints the users current username
 */
void handle_cmd(unsigned char cmd[MAXBUFSIZE], struct Client *client, struct LLClients *clients){
    unsigned char return_msg[MAXBROADCAST];
    
    if (strcmp("help", cmd) == 0){
        strcpy(return_msg, "Available Commands:\n'/users' - list all connected users\n'/myname' - view your current username\n");
    } else if (strcmp("users", cmd) == 0){
        for (struct Client *p = clients->head; p != NULL; p = p->next){
            memset(return_msg, 0, MAXBROADCAST);
            sprintf(return_msg, "Client %d - Name: %s\n", p->sockfd, p->name);
            send(client->sockfd, return_msg, strlen(return_msg), 0);
        }
        return;
    } else if (strcmp("myname", cmd) == 0){
        sprintf(return_msg, "Current Username: %s\n", client->name);
    } else {
        strcpy(return_msg, "Invalid Command. '/help' to list all available commands.\n");
    }

    send(client->sockfd, return_msg, strlen(return_msg), 0);
}

/*
 * handle_send() -- determine how to handle a send given the message type (message, direct message, command)
 * and then execute.
 */
void handle_send(struct Client *client, struct LLClients *clients, int is_direct_message, struct pollfd **pfds, int *fd_count, unsigned char buf[MAXBUFSIZE], int sender_index, int is_cmd)
{
    unsigned char fullmsg[MAXBROADCAST];
    unsigned char adjusted_msg[MAXBUFSIZE];
    unsigned char target_name[MAXCLIENTNAME];
    unsigned char cmd[MAXBUFSIZE];
    int target_sockfd;
    int bytes_sent;

    struct Client *target;

    memset(fullmsg, 0, MAXBROADCAST);
    
    if (is_direct_message == 0 && is_cmd == 0){
        build_message(client, buf, fullmsg);
        broadcast(pfds, *fd_count, fullmsg, sender_index);
        return;
    } else if (is_cmd > 0) {
        strncpy(cmd, (buf+1), is_cmd);
        cmd[is_cmd] = '\0';

        handle_cmd(cmd, client, clients);
        return;
    }

    strncpy(target_name, (buf+1), is_direct_message);
    target_name[is_direct_message] = '\0';

    strncpy(adjusted_msg, buf+2+is_direct_message, MAXBROADCAST-3-is_direct_message);
    build_message(client, adjusted_msg, fullmsg);

    target = find_client_by_name(target_name, clients);
    target_sockfd = target->sockfd;

    if ((bytes_sent = send(target_sockfd, fullmsg, strlen(fullmsg), 0)) == -1){
        perror("server: direct msg\n");
    }
}

/*
 * set_username() -- given a pointer to a client, update their username to the new (given) name
 */
void set_username(struct Client *client, unsigned char buf[MAXBUFSIZE]){
        unsigned char name[MAXCLIENTNAME];
        size_t len = strlen(buf);


        while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n')) {
            buf[len-1] = '\0';
            len--;
        }
        
        if (len > 10){
            strncpy(name, buf, 9);
            name[9] = '\0';
        } else {
            strcpy(name, buf);
        }
        strcpy(client->name, name);
        printf("setting client name: %s\n", name);
        
        return;
}

/*
 * handle_client_data() -- once a socket is ready to read, recv the data and handle operations from there.
 * This includes the listener socket (if ready to read, there is a new client requesting to connect)
 */
void handle_client_data(struct pollfd **pfds, int *fd_count, int *fd_size, int i, struct LLClients *clients)
{
    int bytes_received;
    unsigned char buf[MAXBUFSIZE];
    int sockfd = (*pfds)[i].fd;

    struct Client *client = find_client_by_sockfd(sockfd, clients);

    memset(buf, 0, MAXBUFSIZE);
    
    if ((bytes_received = recv(sockfd, buf, MAXBUFSIZE, 0)) == 0){
        printf("Client disconnected: %s\n", client->name);
        remove_client(clients, client->sockfd);
        remove_from_pfds(pfds, i, fd_count);
    }

    size_t len = strlen(buf);

    if (len > 0){
        // Check if name is set yet

        if (strcmp(client->name, NAMENOTSET) == 0){
            set_username(client, buf);
            return;
        }

        // Name is already set, send message
        int is_direct_msg = is_direct_message(buf);
        int is_cmd = is_command(buf);
        printf("%s: %s", client->name, buf);
        handle_send(client, clients, is_direct_msg, pfds, fd_count, buf, i, is_cmd);
    }
}

/*
 * process_connections() -- actively loop through and poll all connected sockets. When a socket is read,
 * hand off to handle_client_data()
 */
void process_connections(struct pollfd **pfds, int *fd_count, int *fd_size, int listener, struct LLClients *clients)
{
    for (int i = 0; i < *fd_count; i++){
        if ((*pfds)[i].revents & (POLLIN || POLLHUP)){
            // Socket ready for data!
            if ((*pfds)[i].fd == listener){
                int newfd = get_newfd(listener, clients);
                add_to_pfds(pfds, newfd, fd_count, fd_size);
            } else {
                handle_client_data(pfds, fd_count, fd_size, i, clients);
            }
        }
    }
}

/*
 * main() -- handle initial setup, then call main loop (process_connections()). Handle shutdowns
 */
int main(void)
{
    int listener = get_listener();
    int fd_size = 5;
    int fd_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    struct LLClients clients;
    clients.n = 0;
    clients.head = NULL;

    add_to_pfds(&pfds, listener, &fd_count, &fd_size);

    for (;;){
        if (poll(pfds, fd_count, 1000) > 0){
            process_connections(&pfds, &fd_count, &fd_size, listener, &clients);
        }

    }

    return 1;
}
