/*
** server.c -- a strean socket server demo
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// Predefines the PORT we are using
#define PORT "3490" // Port that users will connect to (arbitrary)

// Predefines the max number of connections allowed
#define BACKLOG 10 // How many connections allowed at once.


/*
** sigchld_handler() takes care of child processes when they exit a fork.
** Allows the parent program to "reap" child processes (collect their exit statuses)
** If this handler was not here, child processes would be become zombies and eat up sever resources, 
** eventually overloading the server if enough children are created
**
** waitpid can overwrite errno if errors occur, but we don't want it to affect the main process's errno,
** so we track it before calling waitpid(), then reassign it afterwards, preserving the original errno.
*/
void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning  - Look into this, why is this here?
                                                // It is just to stop warnings

    //waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0); // UNDERSTAND THIS CODE :checkmark:

    errno = saved_errno;
}


/*
** Returns the sockaddr in "in" form, easier for grabbing its values (IP and PORT)
** This way, code works with both IPv4 and IPv6 without needing separate branches for each
*/
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int main(void)
{
    // Listen on sock_fd, new connection on new_fd
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connectors address info
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;


    // Set the hints object to empty to avoid any lingering values
    // Also sets the hints object values as needed, specifying certain criteria
    memset(&hints, 0, sizeof hints);

    // We are using IPv4, so specify AF_INET
    hints.ai_family = AF_INET;
    // We are using Internet Stream Sockets for connections, so specify SOCK_STREAM
    hints.ai_socktype = SOCK_STREAM;
    // We want to use our IP Address, so we set it to passive to automatically do this for us
    hints.ai_flags = AI_PASSIVE; // use my IP

    // First, try to get address info so that we can bind to a port. if this fails, raise an error and quit
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    // Some results may not be 'bindable'? TODO: understand why some might not be. v
    /*
    ** Some results may not be bindable due to various reasons. These include, but are not limited to:
    ** address is already in use by another program
    ** family/protocol is unsupported by this address
    ** permission issues (binding to a port less than 1024 requires root, usually not allowed)
    ** IPv6 issues, dive into this later
    ** others
    **
    ** The goal is the first try to create a socket, then, if successful, bind to it
    ** We use setsockopt() to ensure if the server fails unexpectedly, it can restart immediately
    ** without lingering connection tuples causing errors. Essentially tells the OS we are okay
    ** with binding to the port even if an old process is currently in TIME_WAIT
    ** this does not allow the socket to bind to a port when it is already in use. It just laxes
    ** the strictness of the restart/wait condition. Learn more later :)
    **
    ** Each step is taken sequentially to catch specific errors, only taking the next step
    ** if there were no errors. Very clear and clean.
    */
    for(p = servinfo; p!= NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // TODO: understand setsockopt - why are we doing this?
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    // We already did what we needed with the server info (created socket and binded),
    // so we no longer need this data, free it up!
    freeaddrinfo(servinfo); // all done with this structure (now that we attained socket and binded?)

    // This happens if we could not bind to any sockets, aka no addresses were valid to connect to.
    // Rare, but can happen for various reasons. `p` is the current node in the results from getaddrinfo()
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1); // what is exit(1) - 
                // it is a way to terminate the program immediately and return an error code.
                // 0 means success, any other num is error
    }

    /*
    ** listens for incoming connections so that we can later handle them
    ** this does not stop the program like recv() does, this runs continuously.
    ** this can be stopped with either close() or shutdown(0)
    ** note: shutdown does not close the socket, it stops either/both outgoing or incoming data transfers
    */
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Sets the handler for exiting child processes. See sigchld_handler above
    // This is handled automatically when we clear a child process using exit(0)
    sa.sa_handler = sigchld_handler; // reap all dead processes 
    
    // clears the signal mask so no extra signals are blocked while the handler runs
    sigemptyset(&sa.sa_mask);

    // makes certain interrupted calls auto-restart instead of failing with EINTR
    sa.sa_flags = SA_RESTART;

    // register the sigchld handler. uses this to handle signal children
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");


    /*
    ** Waits for a signal to connect to. accept() is a blocking call, meaning the code stops
    ** until it finishes. This way, the code is stopped until a connection is received,
    ** so this loop only runs while new connections are incoming.
    **
    ** accept() returns a new socket file description to interact with this new connection
    ** sockfd is unaffected, so it can be used to connect with others
    ** new_fd will be used to fork() into a child process. THis is how servers interact
    ** with hundreds/thousands/etc of clients independently 
    */
    while(1) { // main accept loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
    &sin_size); // TODO: understand this code - (struct sockaddr *)&their_add
    if (new_fd == -1) {
        perror("accept");
        continue;
    }

    // 'network to presentation', this converts the ip address to dots and numbers notation to
    // print to the console.
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    /*
    ** Forks the new connection into a child process independently. This is where the bulk of
    ** functionality would be for servers who act on clients independently of other clients.
    ** the foundation of video game servers.
    **
    ** sockfd can be closed because the sockfd is no longer needed in this child fork, this does
    ** NOT affect the parent's sockfd. Many actions can be taken here forward. Eventually, to close
    ** the connection, we use close(new_fd) to close the socket, and exit(). THe parent process will
    ** still need to close the new_fd because it was in a diff scope.
    ** I would like to learn more about this.
    */
    if (!fork()) { // this is the child process TODO: look into this
        close(sockfd); // child does not need the listener - TODO: does this affect the parent process?
        if (send(new_fd,  "Hello, World!\n", 13, 0) == -1)
            perror("send");
        close(new_fd);
        exit(0);
    }
    close(new_fd); // parent does not need this
    }

    return 0;
}