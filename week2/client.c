/*
 * main.c -- client file for handling ciient server logic
 */

// Custom utility files
#include "./utils/player.h"
#include "./utils/buffer_manipulation.h"
#include "./common.h"
#include "./utils/time_custom.h"

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

struct User {
    int x;
    int y;
    int score;
    unsigned char name[MAXUSERNAME];

    struct Treasure *treasure;
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

    unsigned char addr[MAXBUFSIZE];
    inet_ntop(AF_INET, (*p)->ai_addr, addr, (*p)->ai_addrlen);
    printf("setup server socket. sockfd: %d\n", sockfd);
    if ((*p) != NULL){
        printf("%s\n", addr);
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
    if (sendto(sockfd, packet, packet_size, 0, p->ai_addr, p->ai_addrlen) == -1){
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

char* get_x_y_symbol(struct Players *players, int x, int y){
    struct Player *cur = players->head;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->x == x && cur->y == y){
            return PLAYER_SYMBOL2;
        }
    }

    return EMPTY_SYMBOL;
}

void print_gamestate(struct Players *players, struct User *user){
    struct Player *cur = players->head;

    printf("\nTreasure: (%d,%d) - %d point", user->treasure->x, user->treasure->y, user->treasure->value);
    if(user->treasure->value > 1){
        printf("s");
    }
    printf("\n");
    
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

    int treasure_x = user->treasure->x;
    int treasure_y = user->treasure->y;

    if (user->x < 5){
        x_start = 0;
    } else if (user->x > BOUNDX-5){
        x_start = BOUNDX - 9;
    } else {
        x_start = user->x - 5;
    }

    if (user->y < 5){
        y_start = 9;
    } else if (user->y > BOUNDY-5){
        y_start = BOUNDY;
    } else {
        y_start = user->y + 5;
    }

    

    for (int y = y_start; y > y_start-10; y--){
        for (int x = x_start; x < x_start+10; x++){
            if (x==0 || y==0 || x==BOUNDX || y==BOUNDY){
                printf("%s ", BOUNDARY_SYMBOL);
                continue;
            }
            if (x==user->x && y==user->y){
                printf("%s ", PLAYER_SYMBOL);
                continue;
            }
            if (x==treasure_x && y==treasure_y){
                printf("%s ", TREASURE_SYMBOL);
                continue;
            }

            unsigned char *symbol = get_x_y_symbol(players, x, y);

            if (*symbol == 'e'){
                printf("%s ", symbol);
                continue;
            }

            if ((x % 2 == 1 && y % 2 == 1)){
                if ( x % 4 == 1 && y % 4 == 1){
                    printf("%s ", ".");
                    continue;
                }
                printf("%s ", ",");
                continue;
            }

            printf("  ");
        }
        printf("\n");
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

    unsigned char update[MAXBUFSIZE];

    if (input == 'w') user->y++; 
    if (input == 'a') user->x--;
    if (input == 's') user->y--;
    if (input == 'd') user->x++;


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

    //printf("\n=== Current Positions ===\n");
    while (offset < bytes_received){
        int id = unpacki16(buf+offset); offset += 2;
        int x = unpacki16(buf+offset); offset += 2;
        int y = unpacki16(buf+offset); offset += 2;
        int score = unpacki16(buf+offset); offset += 2;

        struct Player *player = get_player_by_id(players, id);
        if (player == NULL){
            // printf("(YOU) %s: (%d, %d) - Score: %d\n", user->name, x, y, score);
            user->x = x;
            user->y = y;
            user->score = score;

            continue;
        }

        player->x = x;
        player->y = y;
        player->score = score;

        //printf("%s: (%d, %d) - Score: %d\n", player->username, x, y, score);
    }

    print_gamestate(players, user);
}

/*
 * handle_user_update() -- handles player info logic - adds new players to players struct, updates usernames, etc.
 */
void handle_user_update(struct Players *players, unsigned char buf[MAXBUFSIZE], int bytes_received){
    int offset = STARTING_OFFSET;
    int id = unpacki16(buf+offset); offset += 2;
    unsigned char username[MAXUSERNAME];

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

    strcpy(player->username, username);
}

/*
 * handle_player_disconnect() -- removes a player from the player struct once the server notifies it that that client has disconnected
 */
void handle_player_disconnect(struct Players *players, unsigned char buf[MAXBUFSIZE]){
    int id = unpacki16(buf+STARTING_OFFSET);
    remove_player(players, id);
}

void handle_treasure(struct User *user, unsigned char buf[MAXBUFSIZE]){
    int offset = STARTING_OFFSET;

    int treasure_x = unpacki16(buf+offset); offset += 2;
    int treasure_y = unpacki16(buf+offset); offset += 2;
    int treasure_value = unpacki16(buf+offset); offset += 2;

    user->treasure->x = treasure_x;
    user->treasure->y = treasure_y;
    user->treasure->value = treasure_value;
}

/*
 * handle_data() -- given data from the server, unpack and determine legitimacy, then determine data type and call corresponding functions
 */
void handle_data(int sockfd, struct addrinfo *p, struct Players *players, struct User *user){
    int bytes_received;
    unsigned char buf[MAXBUFSIZE];

    memset(buf, 0, MAXBUFSIZE);
    bytes_received = recvfrom(sockfd, buf, MAXBUFSIZE, 0, p->ai_addr, &p->ai_addrlen);

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
    }

    if (msg_id == EXIT_ID){
        handle_player_disconnect(players, buf);
    }

    if (msg_id == TREASURE_ID){
        handle_treasure(user, buf);
    }
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

    struct User *user = malloc(sizeof *user);
    user->x = 0;
    user->y = 0;
    user->treasure = malloc(sizeof (struct Treasure));

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
        if (n > 0 && interval_elapsed_cur(last_tick, botmode==1 ? 1 : 100) == 1 && handle_keypress(sockfd, p, input, &original_settings, user, &last_tick) == 0) {
            
            break;
        }

        if (botmode == 1 && interval_elapsed_cur(last_tick, next_move) == 1){
            unsigned char botmove = 'w';
            int random = rand() % 2;

            if (user->x > user->treasure->x){
                if (random == 1 && user->y != user->treasure->y){
                    botmove = user->y > user->treasure->y ?  's' : 'w';
                } else {
                    botmove = 'a';
                }
            } else if (user->x <= user->treasure->x){
                if (random == 1 && user->y != user->treasure->y){
                    botmove = user->y > user->treasure->y ?  's' : 'w';
                } else {
                    botmove = 'd';
                }
            } else {
                botmove = random == 1 ? 'w' : 'd';
            }

            handle_keypress(sockfd, p, botmove, &original_settings, user, &last_tick);
            if (user->score > 5){
                next_move = rand() % 450 + 50;
            } else {
                next_move = rand() % 250 + 50;
            }
        }

        if (poll(pfds, 1, 0) > 0){
            handle_data(sockfd, p, players, user);
        }
    }

    handle_shutdown(sockfd, &original_settings, p);
}