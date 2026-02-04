/*
 * treasure.h -- A utility file for managing 'treasures' in this multiplayer game. stores the id, (x,y) coordinate, and point value of the treasure
 *
 * Functionally similar to client.h
 */

#ifndef TREASURE_H
#define TREASURE_H

#include <stdlib.h>

struct Treasure {
    int id;
    int x;
    int y;
    int value;
    struct Treasure *next;
};

struct Treasures {
    int count;
    struct Treasure *head;
};

void add_treasure(struct Treasures *treasures, struct Treasure *treasure);

void remove_treasure_by_id(struct Treasures *treasures, int id);

void remove_treasure_by_coord(struct Treasures *treasures, int x, int y);

struct Treasure* get_treasure_by_id(struct Treasures *treasures, int id);

struct Treasure* get_treasure_by_coord(struct Treasures *treasures, int x, int y);

#endif