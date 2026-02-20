/*
 * treasure.h -- A utility file for managing 'treasures' in this multiplayer game. stores the id, (x,y) coordinate, and point value of the treasure
 *
 * Functionally similar to client.h
 */

#ifndef TREASURE_H
#define TREASURE_H

#include <stdlib.h>

/*
 * Treasure -- struct for managing a 'treasure' in this multiplayer movement game.
 *
 * id - unique integer identifier for a treasure
 * x - the x position the treasure is located at
 * y - the y position the treasure is located at
 * value - the point value of the treasure (how much it is worth)
 * *next - a pointer to the next treasure, used in a linked list
 */
struct Treasure {
    int id;
    int x;
    int y;
    int value;
    struct Treasure *next;
};

/*
 * Treasures -- linked-list struct for managing multiple 'treasures' in this multiplayer movement game.
 *
 * count - total number of treasures stored in the struct
 * *head - a pointer to the first treasure in the list
 */
struct Treasures {
    int count;
    struct Treasure *head;
};

/*
 * add_treasure() -- add a new treasure struct to the linked list
 */
void add_treasure(struct Treasures *treasures, struct Treasure *treasure);

/*
 * remove_treasure_by_id() -- given an id, remove the corresponding treasure from the linked list if found
 */
void remove_treasure_by_id(struct Treasures *treasures, int id);

/*
 * remove_treasure_by_coord() -- given coordinates, remove the corresponding treasure from the linked list if found
 */
void remove_treasure_by_coord(struct Treasures *treasures, int x, int y);

/*
 * get_treasure_by_id() -- given an id, locate and return the corresponding treasure struct, returns NULL if not found
 */
struct Treasure* get_treasure_by_id(struct Treasures *treasures, int id);

/*
 * get_treasure_by_id() -- given coordinates, locate and return the corresponding treasure struct, returns NULL if not found
 */
struct Treasure* get_treasure_by_coord(struct Treasures *treasures, int x, int y);

#endif