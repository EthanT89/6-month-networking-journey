/*
 * client.c -- client file for handling ciient server logic
 */

// Custom utility files
#include "./utils/player.h"
#include "./utils/buffer_manipulation.h"
#include "./common.h"
#include "./utils/time_custom.h"
#include "./utils/treasure.h"
#include "./proxy_v2/utils/proxy_utils.h"

// Standard socket/server libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>

// Custom terminal logic libraries
#include <termios.h>
#include <fcntl.h>

/*
 * User -- local struct for managing user-specific game state - coordinates, score, username, and a local copy of the current treasures
 *
 * x - current x coordinate of the player
 * y - current y coordinate of the player
 * score - current accumulative score of the player
 * name - the current player's username
 * stale - 0 if printed latest state, 1 if not
 * *treasures - a linked list of all the treasures currently in-game
 */
struct User {
    int id;
    int x;
    int y;
    int score;
    unsigned char name[MAXUSERNAME];
    int stale;

    struct Players *players;
    struct Treasures *treasures;
};

/*
 * set_nonblocking_mode() -- Sets the terminal to non-canonical and non-echoing
 *
 * This was copied from the internet. I did not write this.
 */
void set_nonblocking_mode() { 
    struct termios term_settings;
    tcgetattr(STDIN_FILENO, &term_settings);  // Get current settings

    // Disable canonical mode, echo, and signal processing
    term_settings.c_lflag &= ~(ICANON | ECHO | ISIG);
    term_settings.c_cc[VMIN] = 0;  // Minimum number of characters to read
    term_settings.c_cc[VTIME] = 0; // Timeout in tenths of seconds

    tcsetattr(STDIN_FILENO, TCSANOW, &term_settings);  // Apply changes immediately
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

/*
 * restore_normal_terminal() -- restores the terminal to its default state - non-blocking, waits for 'enter' to be pressed, echoes, etc. 
 */
void restore_normal_terminal(struct termios *original_settings) {
    tcsetattr(STDIN_FILENO, TCSANOW, original_settings);
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

/*
 * get_serversock() -- creates a socket with the server's address info and returns the file descriptor
 */
int get_serversock(struct addrinfo **p){
    int sockfd;
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    unsigned char input[MAXBUFSIZE];

    rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo);
    if (rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through the results and make a socket, no need to bind
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next){

        if ((sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1 ){
            perror("socket");
            continue;
        }
        break;
    }

    // if p = NULL, no valid results
    if ((*p) == NULL){
        fprintf(stderr, "talker: failed to create socket\n");
        exit(1);
    }
    return sockfd;
}

/*
 * construct_update_packet() -- create a coordinate update packet consisting of the app_id, update_id, and x/y coordinates, fills the given buffer
 */
void construct_update_packet(unsigned char packet[MAXBUFSIZE], int x, int y){
    memset(packet, 0, MAXBUFSIZE);  // Initialize buffer to zero
    prepend_i16(packet, y);
    prepend_i16(packet, x);
    prepend_i16(packet, UPDATE_ID);
    prepend_i16(packet, APPID);
}

/*
 * send_packet() -- given the necessary data, send a completed packet to the server
 */
void send_packet(int sockfd, struct addrinfo *p, unsigned char packet[MAXBUFSIZE], int packet_size){
    if (send_proxy(sockfd, packet, packet_size, 0, p->ai_addr, p->ai_addrlen) == -1){
        perror("client: send\n");
        exit(1);
    }
}

/*
 * update_username() -- prompt for new username, update the locally stored username, then send an update to the server
 */
void update_username(int sockfd, struct addrinfo *p, struct termios *original_settings, struct User *user){
    unsigned char new_name[MAXUSERNAME];

    restore_normal_terminal(original_settings);
    printf("Enter new username: ");

    fgets(new_name, MAXUSERNAME, stdin);

    int len = strlen(new_name);
    while (len > 0 && (new_name[len-1] == '\r' || new_name[len-1] == '\n')) {
        new_name[len-1] = '\0';
        len--;
    }

    printf("new name: %s\n", new_name);
    memset(user->name, 0, MAXUSERNAME);
    strcpy(user->name, new_name);

    unsigned char update[MAXBUFSIZE];
    strncpy(update, new_name, MAXUSERNAME);
    prepend_i16(update, UPDATE_USERNAME);
    prepend_i16(update, COMMAND_ID);
    prepend_i16(update, APPID);

    send_packet(sockfd, p, update, 6 + len);
}

/*
 * reset_coords() -- reset local coordinates to (0,0), and send a reset request to the server.
 */
void reset_coords(int sockfd, struct addrinfo *p, struct User *user){
    unsigned char update[MAXBUFSIZE];

    user->x = 0;
    user->y = 0;

    prepend_i16(update, RESET_COORDS);
    prepend_i16(update, COMMAND_ID);
    prepend_i16(update, APPID);

    send_packet(sockfd, p, update, 6);
}

/*
 * print_cmd_help() -- print the available commands to the terminal
 */
void print_cmd_help(){
    printf("\n=== Commands ===\n/help: show available commands\n");
    printf("/updatename: Update your username\n");
    printf("/quit: Quit the program. Can also press 'q'\n");
    printf("/reset: Reset your coordinates to (0,0)\n");
    printf("\n");
}

/*
 * get_x_y_symbol() -- given x,y coordinates, determine if the symbol should represent a player or an empty space.
 *
 * Boundary and Treasure symbols have already been considered up to this point, so only Players and Empty Spaces are left.
 * Currently, the actual print function overrides the empty_symbol for alternating symbols.
 * TODO: clean up logic and code for EMPTY_SYMBOL's
 */

 /*
  * get_x_y_symbol() -- given a coordinate, cross check against current data - treasures, players, and empty spaces, then return
  * the corresponding symbol. Used in tandem with print_gamestate()
  */
char* get_x_y_symbol(struct User *user, struct Players *players, int x, int y){
    struct Player *cur = players->head;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->x == x && cur->y == y){
            printf("\x1b[1m""\x1b[37m");
            return PLAYER_SYMBOL2;
        }
    }

    struct Treasure *treasure = user->treasures->head;
    for (treasure; treasure != NULL; treasure = treasure->next){
        if (treasure->x == x && treasure->y == y){
            printf("\x1b[1m""\x1b[32m");
            return TREASURE_SYMBOL;
        }
    }

    if ((x % 2 == 1 && y % 2 == 1)){
        if ( x % 4 == 1 && y % 4 == 1){
            printf("\x1b[36m");
            return ".";
        }
        printf("\x1b[35m");
        return ",";
    }

    return EMPTY_SYMBOL;
}

/*
 * find_nearest_treasure() -- Given a user struct, extract their coordinates and the coordinates of every active treasure. Then,
 * compare the vector distance from the user to every treasure, and return the treasure with the shortest vector distance.
 */
struct Treasure* find_nearest_treasure(struct User *user){
    // logic to find nearest treasure
    if (user == NULL || user->treasures == NULL || user->treasures->head == NULL){
        return NULL;
    }
    int smallest_distance = -1;
    struct Treasure *nearest = NULL;

    struct Treasure *cur = user->treasures->head;
    for (cur; cur != NULL; cur = cur->next){
        int diff_x = abs(cur->x - user->x);
        int diff_y = abs(cur->y - user->y);
        int rel_distance = sqrt((diff_x*diff_x) + (diff_y*diff_y));

        if (smallest_distance == -1){
            smallest_distance = rel_distance;
            nearest = cur;
            continue;
        }

        if (rel_distance < smallest_distance){
            smallest_distance = rel_distance;
            nearest = cur;
            continue;
        }
    }
    return nearest;
}

/*
 * print_gamestate() -- print the latest gamestate into the structure that follows:
 *
 *  Nearest Treasure: (58,72) - 1 point                  // Nearest treasure coordinates and point value
 *  ________________________________________
 *  |  Player     |   Coords    |  Score   |
 *  |_____________|_____________|__________|
 *  |  ethan      |  (+02,+03)  |  09 pts  |     // Player state
 *  |_____________|_____________|__________|
 *  |  bot        |  (+58,+70)  |  32 pts  |     // Other player's states
 *  |  bot        |  (+08,+37)  |  12 pts  |
 *  |_____________|_____________|__________|
 *
 *  X .   $   .   ,   .                          // Current viewport - an X by X grid of the current state. 
 *  X                                            // Symbols:
 *  X ,   ,   ,   ,   ,                          // 
 *  X                                            // '$' - treasure
 *  X .   ,   e   ,   .                          // 'e' - other player
 *  X                                            // 'o' - current player
 *  X , o ,   ,   ,   ,                          // 'X' - wall/boundary
 *  X                                            // ','/'.' - empty space. Symbols alternate to give illusion of movement 
 *  X .   ,   .   ,   . 
 *  X X X X X X X X X X 
 * 
 */
void print_gamestate(struct Players *players, struct User *user){
    struct Player *cur = players->head;
    struct Treasure *nearest_treasure = find_nearest_treasure(user);

    if (nearest_treasure == NULL){
        printf("\nNearest Treasure: none\n");
    } else {
        printf("\nNearest Treasure: (%d,%d) - %d point", nearest_treasure->x, nearest_treasure->y, nearest_treasure->value);
        if(nearest_treasure->value > 1){
            printf("s");
        }
        printf("\n");
    }
    
    printf("________________________________________\n");
    printf("|  Player     |   Coords    |  Score   |\n");
    printf("|_____________|_____________|__________|\n");
    printf("|  %-6.6s     |  (%+-2.2d,%+-2.2d)  |  %-2.2d pts  |\n", user->name, user->x, user->y, user->score);
    printf("|_____________|_____________|__________|\n");

    if (cur != NULL){
        for (cur; cur != NULL; cur = cur->next){
            printf("|  %-6.6s     |  (%+-2.2d,%+-2.2d)  |  %-2.2d pts  |\n", cur->username, cur->x, cur->y, cur->score);
        }
        printf("|_____________|_____________|__________|\n\n");
    }

    int y_start = 9;
    int x_start = 0;

    int treasure_x = -1;
    int treasure_y = -1;
    if (nearest_treasure != NULL){
        treasure_x = nearest_treasure->x;
        treasure_y = nearest_treasure->y;
    }

    if (user->x < 5){ // User is within 5 units of the leftmost wall.
        x_start = 0;
    } else if (user->x > BOUNDX-5){ // User is within 5 units of the rightmost wall.
        x_start = BOUNDX - 9;
    } else { // User is farther than 5 units from any wall.
        x_start = user->x - 5;
    }

    // Set the initial printing y value. 
    if (user->y < 5){ // User is within 5 units of the bottommost wall.
        y_start = 9;
    } else if (user->y > BOUNDY-5){ // User is within 5 units of the topmost wall.
        y_start = BOUNDY;
    } else { // User is farther than 5 units from any wall.
        y_start = user->y + 5;
    }

    // Loop through every coordinate within the range of y_start to y_start - 10, and x_start to x_start + 10, and print the corresponding symbol
    for (int y = y_start; y > y_start-10; y--){ // Each row
        for (int x = x_start; x < x_start+10; x++){ // Each column within a row
            if (x==0 || y==0 || x==BOUNDX || y==BOUNDY){
                printf("\x1b[40m");
                printf("\x1b[37m""%s ""\x1b[0m", BOUNDARY_SYMBOL);
                continue;
            }
            if (x==user->x && y==user->y){
                //printf("\x1b[47m");
                printf("\x1b[34m""%s ""\x1b[0m", PLAYER_SYMBOL);
                continue;
            }

            printf("\x1b[0m");
            unsigned char *symbol = get_x_y_symbol(user, players, x, y);
            //printf("\x1b[47m");
            printf("%s ""\x1b[0m", symbol);
        }
        printf("\n"); // Jump to next row
    }
}

/*
 * handle_shutdown() -- handles shutting down the client server cleanly. Resets terminal settings, sends exit code to server, and closes the socket.
 */
void handle_shutdown(int sockfd, struct termios *original_settings, struct addrinfo *p){
    unsigned char buf[MAXBUFSIZE];

    // Restore at exit
    tcsetattr(STDIN_FILENO, TCSANOW, original_settings);
    
    memset(buf, 0, MAXBUFSIZE);
    prepend_i16(buf, EXIT_ID);
    prepend_i16(buf, APPID);
    send_packet(sockfd, p, buf, 4);

    close(sockfd);
    exit(1);
}

/*
 * handle_command() -- unpack a command, determine correct command type, then handle command logic
 */
void handle_command(int sockfd, struct addrinfo *p, unsigned char command[MAXCOMMANDSIZE], struct termios *original_settings, struct User *user){

    if (strcmp(command, "/help") == 0){
        print_cmd_help();
        return;
    }

    if (strcmp(command, "/updatename") == 0){
        update_username(sockfd, p, original_settings, user);
        return;
    }

    if (strcmp(command, "/quit") == 0){
        handle_shutdown(sockfd, original_settings, p);
        return;
    }

    if (strcmp(command, "/reset") == 0){
        reset_coords(sockfd, p, user);
        return;
    }

    printf("Unknown command! Use '/help' to see available commands.\n");
}

int validate_movement(char input, struct User *user){
    int y = user->y;
    int x = user->x;

    if (input == 'w') y++; 
    if (input == 'a') x--;
    if (input == 's') y--;
    if (input == 'd') x++;

    // check for player collisions
    struct Player *cur = user->players->head;
    for (cur; cur != NULL; cur = cur->next){
        if (cur->x == x && cur->y == y){
            return 0;
        }
    }

    // check for X bounds
    if (x <= 0 || x >= BOUNDX){
        return 0;
    }
    if (y <= 0 || y >= BOUNDY){
        return 0;
    }

    // check for treasure
    struct Treasure *treasure = user->treasures->head;
    for (treasure; treasure != NULL; treasure = treasure->next){
        if (treasure->x == x && treasure->y == y){
            remove_treasure_by_id(user->treasures, treasure->id);
        }
    }

    user->x = x;
    user->y = y;

    return 1;
}

/*
 * handle_keypress() -- handles keypress logic - commands, quitting, and 'wasd' movements
 */
int handle_keypress(int sockfd, struct addrinfo *p, char input, struct termios *original_settings, struct User *user, int *last_tick){
    if (input == 'q'){
        printf("Thanks for stopping by!\n");
        return 0;
    }

    if (input == 'c'){
        unsigned char commmand[MAXCOMMANDSIZE];

        restore_normal_terminal(original_settings);
        printf("Enter your command...\n");

        fgets(commmand, MAXCOMMANDSIZE, stdin);
        printf("command: %s\n", commmand);

        int len = strlen(commmand);
        while (len > 0 && (commmand[len-1] == '\r' || commmand[len-1] == '\n')) {
            commmand[len-1] = '\0';
            len--;
        }

        handle_command(sockfd, p, commmand, original_settings, user);
        set_nonblocking_mode();
    }

    if (validate_movement(input, user) == 0){
        return 1;
    }

    unsigned char update[MAXBUFSIZE];
    
    user->stale = 1;
    construct_update_packet(update, user->x, user->y);
    send_packet(sockfd, p, update, 8);
    *last_tick = get_time_ms();
    return 1;
}

/*
 * handle_startup() -- initial client server setup - requests user's username and sends first connecting message to the server
 */
void handle_startup(int sockfd, struct addrinfo *p, struct User *user){
    size_t input_len;
    unsigned char input[MAXBUFSIZE];
    
    printf("Welcome to the Multiplayer Movement (Game?)!\nHappy to have you here. Please enter a username: ");
    fgets(input, MAXBUFSIZE - 4, stdin);

    input_len = strlen(input);
    strcat(user->name, input);

    prepend_i16(input, UPDATE_ID);
    prepend_i16(input, APPID);
    send_packet(sockfd, p, input, input_len + 4);
}

/*
 * handle_position_update() -- unpack and handle all positional updates for all players. Prints all coordinate data to the terminal (game state)
 */
void handle_position_update(struct Players *players, unsigned char buf[MAXBUFSIZE], int bytes_received, struct User *user){
    int offset = STARTING_OFFSET;

    while (offset < bytes_received){ // Adaptable to variable amounts of player updates. 
        int id = unpacki16(buf+offset); offset += 2;
        int x = unpacki16(buf+offset); offset += 2;
        int y = unpacki16(buf+offset); offset += 2;
        int score = unpacki16(buf+offset); offset += 2;

        struct Player *player = get_player_by_id(players, id);
        if (id == user->id){
            // user->x = x;
            // user->y = y;
            user->score = score;
            continue;
        }

        if (player == NULL){
            continue;
        }

        player->x = x;
        player->y = y;
        player->score = score;
    }
    // print_gamestate(players, user);
}

/*
 * handle_user_update() -- handles player info logic - adds new players to players struct, updates usernames, etc.
 */
void handle_user_update(struct Players *players, unsigned char buf[MAXBUFSIZE], int bytes_received){
    int offset = STARTING_OFFSET;
    int id = unpacki16(buf+offset); offset += 2;
    unsigned char username[MAXUSERNAME];
    printf("in handle user update!!\n");

    strncpy(username, buf+offset, MAXUSERNAME);
    struct Player *player = get_player_by_id(players, id);

    if (player == NULL){
        printf("adding new player...\n");
        player = malloc(sizeof *player);
        player->id = id;
        strcpy(player->username, username);
        add_player(players, player);
        return;
    }
    printf("okayt\n");

    strcpy(player->username, username);
}

/*
 * handle_player_disconnect() -- removes a player from the player struct once the server notifies it that that client has disconnected
 */
void handle_player_disconnect(struct Players *players, unsigned char buf[MAXBUFSIZE]){
    int id = unpacki16(buf+STARTING_OFFSET);
    remove_player(players, id);
}

/*
 * handle_treasure() -- handle a treasure update from the server
 */
void handle_treasure(struct User *user, unsigned char buf[MAXBUFSIZE]){
    int offset = STARTING_OFFSET;

    int treasure_id = unpacki16(buf+offset); offset += 2;
    int treasure_x = unpacki16(buf+offset); offset += 2;
    int treasure_y = unpacki16(buf+offset); offset += 2;
    int treasure_value = unpacki16(buf+offset); offset += 2;

    if (get_treasure_by_id(user->treasures, treasure_id) != NULL){
        return;
    }

    struct Treasure *new_treasure = malloc(sizeof *new_treasure);

    new_treasure->id = treasure_id;
    new_treasure->x = treasure_x;
    new_treasure->y = treasure_y;
    new_treasure->value = treasure_value;
    add_treasure(user->treasures, new_treasure);
}

/*
 * handle_remove_treasure() -- remove the indicated treasure from the treasures struct given it's ID
 */
void handle_remove_treasure(struct User *user, unsigned char buf[MAXBUFSIZE]){
    int offset = STARTING_OFFSET;

    int treasure_id = unpacki16(buf+offset); offset += 2;
    remove_treasure_by_id(user->treasures, treasure_id);
}

/*
 * handle_error() -- handle any errors sent from the server. Right now, this is limited to a rejected connection
 * due to the server being full. 12 players is the limit at the moment. 
 */
void handle_error(int sockfd, struct User *user, unsigned char buf[MAXBUFSIZE], struct termios *original_settings, struct addrinfo *p){
    int err_code = unpacki16(buf+STARTING_OFFSET);

    if (err_code == REJECT_CONNECTION_ID){
        printf("Connection rejected. Server is full.\n");
        handle_shutdown(sockfd, original_settings, p);
    }

}

void handle_id_update(struct User *user, unsigned char buf[MAXBUFSIZE]){
    int id = unpacki16(buf+STARTING_OFFSET);
    user->id = id;
}

void handle_position_correction(struct User *user, unsigned char buf[MAXBUFSIZE]){
    int offset = STARTING_OFFSET;
    int id = unpacki16(buf+offset); offset += 2;
    int x = unpacki16(buf+offset); offset += 2;
    int y = unpacki16(buf+offset); offset += 2;

    if (user->id != id){
        return;
    }

    user->x = x;
    user->y = y;
    user->stale = 1;
}

/*
 * handle_data() -- given data from the server, unpack and determine legitimacy, then determine data type and call corresponding functions
 */
void handle_data(int sockfd, struct addrinfo *p, struct Players *players, struct User *user, struct termios *original_settings){
    int bytes_received;
    unsigned char buf[MAXBUFSIZE];

    memset(buf, 0, MAXBUFSIZE);
    bytes_received = rec_proxy(sockfd, buf, MAXBUFSIZE, 0, p->ai_addr, &p->ai_addrlen);

    int app_id = unpacki16(buf);
    int msg_id = unpacki16(buf+2);

    if (app_id != APPID){
        printf("ignoring...\n");
        return;
    }

    if (msg_id == UPDATE_ID){
        handle_position_update(players, buf, bytes_received, user);
        return;
    }

    if (msg_id == USERUPDATE_ID){
        handle_user_update(players, buf, bytes_received);
        return;
    }

    if (msg_id == EXIT_ID){
        handle_player_disconnect(players, buf);
        return;
    }

    if (msg_id == NEWTREASURE_ID){
        handle_treasure(user, buf);
        return;
    }

    if (msg_id == DELTREASURE_ID){
        handle_remove_treasure(user, buf);
        return;
    }

    if (msg_id == ERROR_ID){
        handle_error(sockfd, user, buf, original_settings, p);
        return;
    }

    if (msg_id == YOUR_ID_IS){
        handle_id_update(user, buf);
        return;
    }

    if (msg_id == POSITION_CORRECTION_ID){
        handle_position_correction(user, buf);
        return;
    }


    printf("Unknown packet id: \"%d\"\n", msg_id);
}

/*
 * main() -- starts the client server, initializes default values and structs, then runs the startup procedures (get username and 'connect' to server)
 *
 * Main loop: checks for player input (one char at a time, non blocking), calls handle_keypress() if input is found.
 * Loop also checks for incoming data using poll(). If data is ready to be read, calls handle_data()
 * 
 * Lastly, upon program shutdown, calls handle_shutdown()
 */
int main(void)
{
    struct addrinfo *p;
    int sockfd = get_serversock(&p);
    unsigned char buf[MAXBUFSIZE];

    struct Players *players = malloc(sizeof *players);
    players->head = NULL;
    players->total_players = 0;


    struct Treasures *treasures = malloc(sizeof *treasures);
    treasures->count = 0;
    treasures->head = NULL;

    struct User *user = malloc(sizeof *user);
    user->x = BOUNDX/2;
    user->y = BOUNDY/2;
    user->score = 0;
    user->stale = 1;
    user->id = -1;
    memset(user->name, 0, MAXUSERNAME);
    user->players = players;
    user->treasures = treasures;


    struct pollfd *pfds = malloc(sizeof *pfds);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN; // check for ready to read status
    pfds[0].revents = 0;

    // Save original settings before modifying
    struct termios original_settings;
    tcgetattr(STDIN_FILENO, &original_settings);

    handle_startup(sockfd, p, user);

    int botmode = 0;
    int last_tick = 0;
    int next_move = rand() % 500;
    if (strcmp(user->name, "bot\n") == 0){
        printf("botmode on!\n");
        botmode = 1;
    }

    set_nonblocking_mode();

    int len = strlen(user->name);
    while (len > 0 && (user->name[len-1] == '\r' || user->name[len-1] == '\n')) {
        user->name[len-1] = '\0';
        len--;
    }

    
    while (1){
        char input;
        int n = read(STDIN_FILENO, &input, 1);
        if (n > 0 && interval_elapsed_cur(last_tick, botmode==1 ? 1 : 1) == 1 && handle_keypress(sockfd, p, input, &original_settings, user, &last_tick) == 0) {
            break;
        }

        if (botmode == 1 && interval_elapsed_cur(last_tick, next_move) == 1){
            unsigned char botmove = 'w';
            int random = rand() % 2;

            struct Treasure *treasure = find_nearest_treasure(user);
            if (treasure == NULL){
                continue;
            }

            if (user->x > treasure->x){
                if (random == 1 && user->y != treasure->y){
                    botmove = user->y > treasure->y ?  's' : 'w';
                } else {
                    botmove = 'a';
                }
            } else if (user->x < treasure->x){
                if (random == 1 && user->y != treasure->y){
                    botmove = user->y > treasure->y ?  's' : 'w';
                } else {
                    botmove = 'd';
                }
            } else {
                if (user->y > treasure->y){
                    botmove = 's';
                } else {
                    botmove = 'w';
                }
            }

            handle_keypress(sockfd, p, botmove, &original_settings, user, &last_tick);
            next_move = rand() % 450 + 50;
        }

        if (user->stale == 1 && interval_elapsed_cur(last_tick, 0) == 1){
            print_gamestate(players, user);
            last_tick = get_time_ms();
            user->stale = 0;
        }

        if (poll(pfds, 1, 0) > 0){
            handle_data(sockfd, p, players, user, &original_settings);
            user->stale = 1;
        }
    }

    handle_shutdown(sockfd, &original_settings, p);
}