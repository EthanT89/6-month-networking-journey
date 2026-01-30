/*
 * main.c -- main file for handling game server logic
 */

#include "./utils/pfds.h"
#include "./utils/time_custom.h"
#include "./utils/player.h"
#include "./utils/buffer_manipulation.h"
#include "./common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>

int get_socket(){
    int sockfd, rv;
    int yes = 1;
    struct addrinfo hints, *results, *p;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // first, use getaddrinfo() to find some addresses!
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &results)) != 0){
        perror("server: getaddrinfo\n");
        exit(1);
    }

    for (p = results; p != NULL; p = p->ai_next){
        // first, try to make a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket\n");
            continue;
        }

        // no more "address already in use" error :)
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server: bind\n");
            continue;
        }

        printf("Successfully created socket!\n");
        break;
    }

    if (p == NULL){
        perror("server: start\n");
        exit(1);
    }

    // if (listen(sockfd, MAXBACKLOG) == -1){
    //     perror("server: listen\n");
    //     exit(1);
    // }

    return sockfd;
}

void construct_user_update_packet(struct Player *player, unsigned char packet[MAXBUFSIZE]){
    packi16(packet, APPID);
    packi16(packet+2, USERUPDATE_ID);
    packi16(packet+4, player->id);
    strcpy(packet+6, player->username);
    printf("username: %s\n", packet+6);
}

void send_user_update_all(int sockfd, struct Players *players, struct Player *player){
    unsigned char update[MAXBUFSIZE];
    construct_user_update_packet(player, update);
    
    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        sendto(sockfd, update, strlen(update+6)+6, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }
}

void send_all_users_data(int sockfd, struct Players *players, struct Player *player){
    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        unsigned char update[MAXBUFSIZE];
        construct_user_update_packet(cur, update);

        sendto(sockfd, update, strlen(update+6)+6, 0, (struct sockaddr*)&player->addr, player->addrlen);
    }
}

void handle_command(){
    // TODO: handle command (future)
}

void handle_new_connection(int sockfd, struct Players *players, struct sockaddr_in addr, socklen_t addr_len, unsigned char data[MAXBUFSIZE], int *id_count){
    
    struct Player *new_player = malloc(sizeof *new_player);
    new_player->addr = addr;
    new_player->addrlen = addr_len;
    new_player->id = (*id_count)++;
    new_player->x = 0;
    new_player->y = 0;

    // extract username
    size_t len = strlen(data);
    if (len > MAXUSERNAME){
        len = MAXUSERNAME;
    }

    if (len <= 0){
        return;
    }
    
    // Get rid of any remaining escape sequences to clean up the username
    for (int i = 0; i < len; i++){
        if (data[i] == '\n' || data[i] == '\r'){
            data[i] = '\0';
        }
    }

    strncpy(new_player->username, data, len);
    send_user_update_all(sockfd, players, new_player);
    send_all_users_data(sockfd, players, new_player);

    add_player(players, new_player);
}

void handle_update(struct Player *player, unsigned char data[MAXBUFSIZE]){
    int x = unpacki16(data);
    int y = unpacki16(data + 2);

    player->x = x;
    player->y = y;
}

void handle_disconnection(struct Players *players, int id){
    remove_player(players, id);
}

void handle_data(int sockfd, struct Players *players, int *id_count){

    struct sockaddr_in *their_addr = malloc(sizeof *their_addr);
    socklen_t their_len = sizeof *their_addr;
    unsigned char data[MAXBUFSIZE];
    int bytes_received;

    memset(data, 0, MAXBUFSIZE);
    if ((bytes_received = recvfrom(sockfd, data, MAXBUFSIZE, 0, (struct sockaddr*)their_addr, &their_len)) == -1){
        perror("server: recvfrom\n");
        exit(1);
    }

    int app_id = unpacki16(data);
    memmove(data, data+2, MAXBUFSIZE-2); // "shifts" the contents of data to the left by two bytes
    int msg_type = unpacki16(data);
    memmove(data, data+2, MAXBUFSIZE-2);

    if (app_id != APPID){
        printf("ignoring message.\n");
        return;
    }

    // Map sender to connected players, if no mapping is found, the sender is a new client
    struct Player *sender = get_player_by_addr(players, *their_addr);
    if (sender == NULL){
        if (msg_type == UPDATE_ID){
            printf("new connection!\n");
            handle_new_connection(sockfd, players, *their_addr, their_len, data, id_count);
        }
        return;
    }

    if (msg_type == UPDATE_ID){
        handle_update(sender, data);
        return;
    }

    if (msg_type == COMMAND_ID){
        printf("received command!\n");
        return;
    }

    if (msg_type == EXIT_ID){
        printf("%s disconnected!\n", sender->username);
        handle_disconnection(players, sender->id);
        return;
    }
}

void broadcast_positions(int sockfd, struct Players *players){
    unsigned char update[MAXBUFSIZE];
    memset(update, 0, MAXBUFSIZE);

    // Pack appid and updateid to secure message
    packi16(update, APPID);
    packi16(update+2, UPDATE_ID);

    int offset = 4;
    struct Player *cur = players->head;

    // Loop through players and pack their x,y coordinates and ID
    for (cur; cur != NULL; cur = cur->next){
        packi16(update+offset, cur->id);
        offset += 2;
        packi16(update+offset, cur->x);
        offset += 2;
        packi16(update+offset, cur->y);
        offset += 2;
    }    

    cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        sendto(sockfd, update, offset, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }
}

int main(void)
{
    int last_tick = 0;
    int sockfd = get_socket();
    int id_count = 1;

    struct pollfd *pfds = malloc(sizeof *pfds);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN; // check for ready to read status
    pfds[0].revents = 0;

    struct Players *players = malloc(sizeof *players);
    players->total_players = 0;
    players->head = NULL;

    while (1)
    {
        if (interval_elapsed_cur(last_tick, TICKRATE)){
            // handle tick logic - broadcast()
            // Assume all packets make it. Packets are time sensitive so it is not worth it to resend. (locally, this is extremely fast though)
            broadcast_positions(sockfd, players);
            last_tick = get_time_ms();
        }

        // Received some kind of data
        if (poll(pfds, 1, 0) == 1){
            handle_data(sockfd, players, &id_count); // handle data
        }        
    }
}


/*
 * FAILING DISCONNECT CASES (a connects, then b, then c):
 *
 * a -> c (extra unknown user) -> (crashes)  ---  C does not get removed correctly (first in linked list)
 * c -> (not removed correctly) --- First node must not be getting removed correct, still present.
 * 
 */