/*
 * main.c -- client file for handling ciient server logic
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
#include <arpa/inet.h>

#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <termios.h>
#include <fcntl.h>

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

void construct_update_packet(unsigned char packet[MAXBUFSIZE], int x, int y){
    memset(packet, 0, MAXBUFSIZE);  // Initialize buffer to zero
    prepend_i16(packet, y);
    prepend_i16(packet, x);
    prepend_i16(packet, UPDATE_ID);
    prepend_i16(packet, APPID);
}

void construct_packet_headers(unsigned char packet[MAXBUFSIZE]){
    prepend_i16(packet, UPDATE_ID);
    prepend_i16(packet, APPID);
}

void send_packet(int sockfd, struct addrinfo *p, unsigned char packet[MAXBUFSIZE], int packet_size){
    if (sendto(sockfd, packet, packet_size, 0, p->ai_addr, p->ai_addrlen) == -1){
        perror("client: send\n");
        exit(1);
    }
}

int handle_keypress(int sockfd, struct addrinfo *p, char input, int *x, int *y){
    if (input == 'q'){
        printf("Thanks for stopping by!\n");
        return 0;
    }

    unsigned char update[MAXBUFSIZE];

    if (input == 'w') (*y)++; 
    if (input == 'a') (*x)--;
    if (input == 's') (*y)--;
    if (input == 'd') (*x)++;


    construct_update_packet(update, *x, *y);
    send_packet(sockfd, p, update, 8);
    return 1;
}

void handle_startup(int sockfd, struct addrinfo *p, unsigned char myname[MAXUSERNAME]){
    size_t input_len;
    unsigned char input[MAXBUFSIZE];
    
    printf("Welcome to the Multiplayer Movement (Game?)!\nHappy to have you here. Please enter a username: ");
    fgets(input, MAXBUFSIZE - 4, stdin);

    input_len = strlen(input);
    strncat(myname, input, MAXUSERNAME);

    construct_packet_headers(input);
    send_packet(sockfd, p, input, input_len + 4);

}

void handle_position_update(struct Players *players, unsigned char buf[MAXBUFSIZE], int bytes_received, unsigned char myname[MAXUSERNAME]){
    int offset = STARTING_OFFSET;

    printf("\n=== Current Positions ===\n");
    while (offset < bytes_received){
        int id = unpacki16(buf+offset); offset += 2;
        int x = unpacki16(buf+offset); offset += 2;
        int y = unpacki16(buf+offset); offset += 2;

        struct Player *player = get_player_by_id(players, id);
        if (player == NULL){
            printf("YOU: (%d, %d)\n", x, y);
            continue;
        }

        player->x = x;
        player->y = y;


        printf("%s: (%d, %d)\n", player->username, x, y);
    }
}

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

void handle_data(int sockfd, struct addrinfo *p, struct Players *players, unsigned char myname[MAXUSERNAME]){
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
        handle_position_update(players, buf, bytes_received, myname);
        return;
    }

    if (msg_id == USERUPDATE_ID){
        handle_user_update(players, buf, bytes_received);
    }
}

int main(void)
{
    struct addrinfo *p;
    int sockfd = get_serversock(&p);
    unsigned char myname[MAXUSERNAME];
    unsigned char buf[MAXBUFSIZE];

    struct Players *players = malloc(sizeof *players);
    players->head = NULL;
    players->total_players = 0;

    int x = 0;
    int y = 0;

    struct pollfd *pfds = malloc(sizeof *pfds);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN; // check for ready to read status
    pfds[0].revents = 0;

    // Save original settings before modifying
    struct termios original_settings;
    tcgetattr(STDIN_FILENO, &original_settings);

    handle_startup(sockfd, p, myname);
    set_nonblocking_mode();

    while (1){
        char input;
        int n = read(STDIN_FILENO, &input, 1);
        if (n > 0 && handle_keypress(sockfd, p, input, &x, &y) == 0) {
            break;
        }

        if (poll(pfds, 1, 0) > 0){
            handle_data(sockfd, p, players, myname);
        }
    }

    // Send exit message to indicate disconnection
    memset(buf, 0, MAXBUFSIZE);
    prepend_i16(buf, EXIT_ID);
    prepend_i16(buf, APPID);
    send_packet(sockfd, p, buf, 4);

    // Restore at exit
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
    close(sockfd);
}