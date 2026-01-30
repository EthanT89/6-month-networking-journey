

#include "./player.h"

int main()
{
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <string.h>

    struct sockaddr_in server_addr;

    // Initialize the structure to zero
    memset(&server_addr, 0, sizeof(server_addr));

    // Set the address family to IPv4
    server_addr.sin_family = AF_INET;

    // Set the port in network byte order (host-to-network short)
    server_addr.sin_port = htons(8080);

    // Set the IP address (convert from string to binary)
    inet_pton(AF_INET, "10.0.0.21", &server_addr.sin_addr);   

    struct Players *players = malloc(sizeof (struct Players));
    struct Player *player = malloc(sizeof (struct Player));
    struct Player *player2 = malloc(sizeof (struct Player));
    struct Player *player3 = malloc(sizeof (struct Player));
    struct Player *player4 = malloc(sizeof (struct Player));

    unsigned char playername[MAXUSERNAME] = "myuser";
    unsigned char falsename[MAXUSERNAME] = "nonono!";
    printf("created player instances...\n");


    memset(player, 0, sizeof player);
    printf("1\n");
    memset(player2, 0, sizeof *player2);
    printf("2\n");
    memset(player3, 0, sizeof *player3);
    printf("3\n");
    memset(player4, 0, sizeof *player4);
    printf("4\n");
    memset(players, 0, sizeof *players);

    printf("memset complete...\n");

    player->id = 1;
    player2->id = 2;
    player3->id = 3;
    player4->sockaddr = server_addr;
    strcpy(player4->username, falsename);
    strcpy(player->username, playername);
    player4->id = 4;

    printf("adding players...\n");

    add_player(players, player);
    add_player(players, player2);
    add_player(players, player3);

    printf("1: %d\n", players->head->id);
    printf("2: %d\n", players->head->next->id);
    printf("3: %d\n", players->head->next->next->id);
    printf("count: %d\n", players->total_players);

    printf("removing player...\n");
    remove_player(players, player2->id);
    printf("count: %d\n", players->total_players);

    printf("1: %d\n", players->head->id);
    printf("2: %d\n", players->head->next->id);

    add_player(players, player4);

    struct Player *search = get_player_by_addr(players, server_addr);

    printf("search result 1: %d\n", search->id); // should return 4

    struct Player *search2 = get_player_by_id(players, 3);

    printf("search result 2: %d\n", search2->id); // should return 3

    struct Player *search3 = get_player_by_name(players, playername);

    printf("search result 3: %d\n", search3->id); // should return 1
}