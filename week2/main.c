/*
 * main.c -- main file for handling game server logic
 */

// Custom utility files
#include "./utils/time_custom.h"
#include "./utils/player.h"
#include "./utils/buffer_manipulation.h"
#include "./common.h"

// Standard socket/server libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>

/*
 * gen_new_treasure() -- populates a treasure struct with random x,y coordinates and a score value. x,y are bounded within BOUNDX and BOUNDY
 * value is bounded 1-3 points.
 */
void gen_new_treasure(struct Treasure *treasure){
    srand(time(NULL));
    treasure->x = (rand() % (BOUNDX-2)) + 1;
    treasure->y = (rand() % (BOUNDY-2)) + 1;

    treasure->value = (rand() % (MAXTREASUREVAL-1)) + 1;
}

/*
 * broadcast_treasure() -- broadcast the data of a treasure to all players to update the game state. x, y, and points values are sent.
 */
void broadcast_treasure(int sockfd, struct Players *players, struct Treasure *treasure){
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, TREASURE_ID); offset += 2;
    packi16(update+offset, treasure->x); offset += 2;
    packi16(update+offset, treasure->y); offset += 2;
    packi16(update+offset, treasure->value); offset += 2;

    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        sendto(sockfd, update, offset, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }
}

/*
 * get_socket() -- create a socket and return its file descriptor
 */
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

/*
 * construct_user_update_packet() -- create a packet containing info about a particular user
 */
void construct_user_update_packet(struct Player *player, unsigned char packet[MAXBUFSIZE]){
    packi16(packet, APPID);
    packi16(packet+2, USERUPDATE_ID);
    packi16(packet+4, player->id);
    strcpy(packet+6, player->username);
}

/*
 * send_all_users_data() -- send a SINGLE player's info to ALL players
 */
void send_user_update_all(int sockfd, struct Players *players, struct Player *player){
    unsigned char update[MAXBUFSIZE];
    construct_user_update_packet(player, update);
    
    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        if (cur->id == player->id){
            continue;
        }
        sendto(sockfd, update, strlen(update+6)+6, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }
}

/*
 * send_user_update_all() -- send EVERY players' info to ONE player.
 */
void send_all_users_data(int sockfd, struct Players *players, struct Player *player){
    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        unsigned char update[MAXBUFSIZE];
        construct_user_update_packet(cur, update);
        sendto(sockfd, update, strlen(update+6)+6, 0, (struct sockaddr*)&player->addr, player->addrlen);
    }
}

/*
 * handle_command() -- unpack and handle any incoming commands. Ignore unhandled commands.
 *
 * Available commands:
 * 'update_username' - update an existing user's username, then broadcast this update to all connected users
 * 'reset_coords' - reset an existing user's coordinates to (0,0), and broadcast this update to all connected users
 */
void handle_command(struct Players *players, struct Player *player, unsigned char command[MAXBUFSIZE], int sockfd){
    // TODO: handle command (future)
    int command_id = unpacki16(command);

    if (command_id == UPDATE_USERNAME){
        printf("updating username: %s\n", command+2);
        strncpy(player->username, command+2, MAXUSERNAME);

        send_user_update_all(sockfd, players, player);
        return;
    }

    if (command_id == RESET_COORDS){
        printf("resetting coords for %s\n", player->username);
        player->x = rand() % BOUNDX-2;
        player->y = rand() % BOUNDY-2;
        return;
    }

}

/*
 * handle_new_connection() -- create a new player and assign the new connection to this player. Update their username, id, coords, etc.
 *
 * Then, send all existing player info about every connected player to this new player, so they have the latest game state. Lastly, send the
 * new user's info to all connected users.
 */
void handle_new_connection(int sockfd, struct Players *players, struct sockaddr_in addr, socklen_t addr_len, unsigned char data[MAXBUFSIZE], int *id_count, struct Treasure *treasure){
    
    struct Player *new_player = malloc(sizeof *new_player);
    new_player->addr = addr;
    new_player->addrlen = addr_len;
    new_player->id = (*id_count)++;
    new_player->score = 0;
    new_player->x = (rand() % BOUNDX-2) + 1;
    new_player->y = (rand() % BOUNDY-2) + 1;

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
    broadcast_treasure(sockfd, players, treasure);
}

/*
 * validate_movement() -- given a player's requested updated coordinates, validate against possible cheats, walls, and other players
 *
 * If a player tries to move more than 2 units at a time (room for one packet drop), it is considered cheating and their old coordinates
 * are sent back, forcing them to reset.
 * If a player tries to move into a wall, the same process ensues
 * If a player tries to move into a unit occupied by another player, also rejected
 */
int validate_movement(int sockfd, struct Players *players, struct Player *player, int x, int y){
    int valid = 1;

    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        if (cur->id == player->id){
            continue;
        }
        if (cur->x == x && cur->y == y){ // Other player occupies that tile
            valid = 0;
            break;
        }
    }

    if (valid == 1 && (abs(x - player->x) > 2 || abs(y - player->y) > 2)){ // Too many movements at once (cheating)
        valid = 0;
    } else if (valid == 1 && (x == 0 || x == BOUNDX || y == 0 || y == BOUNDY)){ // Out of bounds
        valid = 0;
    }
    
    if (valid == 0){
        unsigned char update[MAXBUFSIZE];
        memset(update, 0, MAXBUFSIZE);

        // Pack appid and updateid to secure message
        packi16(update, APPID);
        packi16(update+2, UPDATE_ID);

        int offset = 4;

        packi16(update+offset, player->id);
        offset += 2;
        packi16(update+offset, player->x);
        offset += 2;
        packi16(update+offset, player->y);
        offset += 2;

        sendto(sockfd, update, offset, 0, (struct sockaddr*)&player->addr, player->addrlen);

        return 0;
    }
    return 1;
}

/*
 * handle_update() -- handle an update from a particular player
 *
 * Unpacks the x and y coordinates of the update, and verifies it is legitimate by checking the difference between the new coordinates and old coordinates.
 * x/y coordinates can change by 2 units at a time to allow for possible missed packets, otherwise it is considered illegitimate.
 * 
 * If an update is considered illegitimate, the old coordinates are sent to the sender, forcing them to fallback to these values.
 */
void handle_update(struct Players *players, struct Player *player, unsigned char data[MAXBUFSIZE], int sockfd, struct Treasure *treasure){
    int x = unpacki16(data);
    int y = unpacki16(data + 2);

    // Check for valid x,y change (player can only move one unit at a time.)
    if (validate_movement(sockfd, players, player, x, y) == 0){
        return;
    }

    player->x = x;
    player->y = y;

    if (x == (treasure->x) && (y == treasure->y)){
        printf("%s got a treasure worth %d points!\n", player->username, treasure->value);
        player->score += treasure->value;
        gen_new_treasure(treasure);
        broadcast_treasure(sockfd, players, treasure);
    }
}

/*
 * handle_disconnection() -- cleanly handle a disconnection. Remove this player from the players struct and update all connected users of this change. This relies
 * on the client server sending an exit code (assumes clean exit on their end.)
 * 
 * Currently does not handle unintentional connections.
 */
void handle_disconnection(int sockfd, struct Players *players, int id){
    remove_player(players, id);

    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, EXIT_ID); offset += 2;
    packi16(update+offset, id); offset += 2;

    struct Player *cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        sendto(sockfd, update, offset, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }
}

/*
 * handle_data() -- Given ANY data to read from the server socket, unpack, verify, and handle next actions for the data.
 */
void handle_data(int sockfd, struct Players *players, int *id_count, struct Treasure *treasure){

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
            handle_new_connection(sockfd, players, *their_addr, their_len, data, id_count, treasure);
        }
        return;
    }

    if (msg_type == UPDATE_ID){
        handle_update(players, sender, data, sockfd, treasure);
        return;
    }

    if (msg_type == COMMAND_ID){
        printf("received command!\n");
        handle_command(players, sender, data, sockfd);
        return;
    }

    if (msg_type == EXIT_ID){
        printf("%s disconnected!\n", sender->username);
        handle_disconnection(sockfd, players, sender->id);
        return;
    }
}

/*
 * broadcast_updates() -- Create an update packet containing the x,y coordinates and score for ALL players, and send that packet to every connected player.
 * If an update is identical to the last update, no update is sent, as that would be redundant
 */
void broadcast_updates(int sockfd, struct Players *players, unsigned char last_update[MAXBUFSIZE]){
    unsigned char update[MAXBUFSIZE];
    memset(update, 0, MAXBUFSIZE);
    int offset = 0;

    // Pack appid and updateid to secure message
    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, UPDATE_ID); offset += 2;

    struct Player *cur = players->head;

    // Loop through players and pack their x,y coordinates and ID
    for (cur; cur != NULL; cur = cur->next){
        packi16(update+offset, cur->id); offset += 2;
        packi16(update+offset, cur->x); offset += 2;
        packi16(update+offset, cur->y); offset += 2;
        packi16(update+offset, cur->score); offset += 2;
    }

    if (memcmp(last_update, update, MAXBUFSIZE) == 0){
        return;
    }

    cur = players->head;
    for (cur; cur != NULL; cur = cur->next){
        sendto(sockfd, update, offset, 0, (struct sockaddr*)&cur->addr, cur->addrlen);
    }

    memcpy(last_update, update, MAXBUFSIZE);
}

/*
 * main() -- Startup the multiplayer movement server, setup initial structs, and start main loop
 *
 * Main loop constantly checks again the last tick time, when the last tick time is greater or equal to TICKRATE seconds ago,
 * updates are broadcasted to all users.
 * 
 * It also looks for any incoming data using poll(). If any data is ready to be read, it calls handle_data() 
 */
int main(void)
{
    int last_tick = 0;
    int sockfd = get_socket();
    int id_count = 1;

    struct Treasure *treasure = malloc(sizeof *treasure);
    gen_new_treasure(treasure);

    unsigned char last_update[MAXBUFSIZE];

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
            broadcast_updates(sockfd, players, last_update);
            last_tick = get_time_ms();
        }

        // Received some kind of data
        if (poll(pfds, 1, 0) == 1){
            handle_data(sockfd, players, &id_count, treasure); // handle data
        }        
    }
}
