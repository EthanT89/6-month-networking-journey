#include "./player.h"

void add_player(struct Players *players, struct Player *player){
    if (players->total_players == 0){
        players->head = player;
        ++(players->total_players);
        return;
    }

    player->next = players->head;
    players->head = player;
    ++(players->total_players);
}

void remove_player(struct Players *players, int id){
    if (players->total_players == 0){
        return;
    }
    if (players->total_players == 1){
        free(players->head);
        players->head = NULL;
        --(players->total_players);
        return;
    }

    struct Player *cur = players->head;
    struct Player *prev = NULL;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->id == id){
            if (prev == NULL){
                players->head = cur->next;
            } else {
                prev->next = cur->next;
            }

            --(players->total_players);
            free(cur);
            return;
        }
        prev = cur;
    }
}

struct Player* get_player_by_addr(struct Players *players, struct sockaddr_in sockaddr){
    struct Player *p = players->head;

    for (p; p != NULL; p = p->next){
        if (p->addr.sin_addr.s_addr == sockaddr.sin_addr.s_addr && p->addr.sin_port == sockaddr.sin_port){
            return p;
        }
    }

    return NULL;
}

struct Player* get_player_by_id(struct Players *players, int id) {
    struct Player *p = players->head;

    for (p; p != NULL; p = p->next){
        if (p->id == id){
            return p;
        }
    }

    return NULL;
}

struct Player* get_player_by_name(struct Players *players, unsigned char name[MAXUSERNAME]) {
    struct Player *p = players->head;

    for (p; p != NULL; p = p->next){
        if (strcmp(name, p->username) == 0){
            return p;
        }
    }

    return NULL;
}
