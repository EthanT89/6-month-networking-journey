/*
 * treasure.c -- utility logic for treasure management
 */

 #include "./treasure.h"

void add_treasure(struct Treasures *treasures, struct Treasure *treasure){
    if (treasures->count == 0){
        treasure->next = NULL;
        treasures->head = treasure;
        treasures->count++;
        return;
    }

    treasure->next = treasures->head;
    treasures->head = treasure;
    treasures->count++;
}

void remove_treasure_by_id(struct Treasures *treasures, int id){
    if (treasures->count == 0){
        return;
    }


    if (treasures->count == 1){
        free(treasures->head);
        treasures->head = NULL;
        --(treasures->count);
        return;
    }

    struct Treasure *cur = treasures->head;
    struct Treasure *prev = NULL;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->id == id){
            if (prev == NULL){
                treasures->head = cur->next;
            } else {
                prev->next = cur->next;
            }

            --(treasures->count);
            free(cur);
            return;
        }
        prev = cur;
    } 
}

void remove_treasure_by_coord(struct Treasures *treasures, int x, int y){
    if (treasures->count == 0){
        return;
    }

    if (treasures->count == 1){
        free(treasures->head);
        treasures->head = NULL;
        --(treasures->count);
        return;
    }

    struct Treasure *cur = treasures->head;
    struct Treasure *prev = NULL;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->x == x && cur->y == y){
            if (prev == NULL){
                treasures->head = cur->next;
            } else {
                prev->next = cur->next;
            }

            --(treasures->count);
            free(cur);
            return;
        }
        prev = cur;
    } 
}

struct Treasure* get_treasure_by_id(struct Treasures *treasures, int id){
    struct Treasure *cur = treasures->head;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->id == id){
            return cur;
        }
    }

    return NULL;
}

struct Treasure* get_treasure_by_coord(struct Treasures *treasures, int x, int y){
    struct Treasure *cur = treasures->head;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->x == x && cur->y == y){
            return cur;
        }
    }

    return NULL;
}