/*
 * player.h --util file for managing players in the player location game
 */
#ifndef PLAYER_H
#define PLAYER_H

#include "../common.h"
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Player -- A struct for storing connected user data.
 *
 * id -- unique user id to manage users
 * username -- user chosen username used as a display name
 * sockaddr -- unique addr assigned on connection that helps match message to sender
 * x -- latest x position
 * y -- latest y position
 * *next -- pointer to the next player in the linked list
 */
struct Player {
    unsigned int id;
    unsigned char username[MAXUSERNAME];

    struct sockaddr_in addr;
    socklen_t addrlen;

    int x;
    int y;

    struct Player* next;
};

/*
 * Players -- A linked-list-based struct for storing and managing multiple Player(s)
 *
 * total_players -- the total number of active players
 * *head -- a pointer to the first Player in the linked list
 */
struct Players {
    int total_players;
    struct Player* head;
};

/*
 * add_player() -- add a player to the given players structure
 */
void add_player(struct Players *players, struct Player *player);

/*
 * remove_player() -- given an id, remove the corresponding player from the Players linked list
 */
void remove_player(struct Players *players, int id);

/*
 * get_player_by_*() -- identify and return the pointer to the target Player struct, given one of three possible search terms
 */
struct Player* get_player_by_id(struct Players *players, int id);

/*
 * get_player_by_*() -- identify and return the pointer to the target Player struct, given one of three possible search terms
 */
struct Player* get_player_by_name(struct Players *players, unsigned char name[MAXUSERNAME]);

/*
 * get_player_by_*() -- identify and return the pointer to the target Player struct, given one of three possible search terms
 */
struct Player* get_player_by_addr(struct Players *players, struct sockaddr_in sockaddr);

#endif