# TCP Chat Server Setup and Usage

## Get it goin!

### It's not too hard...

To get the chat server running, you only need to know one command: `./chat`. Just type that up in the console and the server will be running!

To connect a client using telnet (or any other client program you may have made), use the localhost address and port 1209. Side note, 12/09 is my birthday! Feel free to change the port number in `common.h` if you don't like my birthday. To make it easy, here is the telnet command: `telnet localhost 1209`. 

Once a client is connected, type in your desired username then send messages to your heart's content!

See the next session for some details about commands and direct messaging.

## So... How does this all work?

### Understanding File Structure

If you really want to understand what's going on, and possibly make further enhancements, then it would do you justice to understand what each file does.

So, lets talk file structure!

The main server file is `chat_server.c`. This handles the bulk of server processing and logic. It utilizes multiple utility files to carry out it's functions, such as `utils/client.c`, `utils/pfds.c`, and `common.h`.

`common.h` houses all global (common) variables, like port number, max values, and more. The file explains each variable a bit further.

`utils/client.c` handles all client structure logic. The `struct Client` and `struct LLClients` are declared there, as well as any function relating to the handling of these two structs. Once again, the file contains deeper explanations for it's purpose.

`utils/pfds.c` contains a few helper functions to manage the `pfds` (poll file descriptors). the `pfds` is used for storing client socket file descriptors to be used by `poll()`. `poll()` allows for the handling of multiple clients.

It may seem redundant to have two different data structures and utility files for handling clients; however, the desire for custom functionality demands for this. `poll()` makes multiclient functionality easier by `poll()`'ing the connected sockets without blocking the program. On the flip side, the `struct Client` data structure allows for custom features like usernames. More to come, but it lets us take the first step.

### Custom Commands

Users can send custom commands like `/users` and `/help` by prepending the command with "/". The program parses each received message and checks for certain symbols. If "/" is present, the message is seen as a command and handled by the appropriately named `handle_cmd()`. 

To add new commands, just add a new if branch to the `handle_cmd` logic structure and choose how to handle that command from there! For example, if I wanted to add the command "/quit", I would simply add the following code:

```c
void handle_cmd(unsigned char cmd[MAXBUFSIZE], struct Client *client, struct LLClients *clients){
    unsigned char return_msg[MAXBROADCAST];
    
    if (strcmp("help", cmd) == 0){
        strcpy(return_msg, "Available Commands:\n'/users' - list all connected users\n'/myname' - view your current username\n");
    } else if (strcmp("users", cmd) == 0){
        for (struct Client *p = clients->head; p != NULL; p = p->next){
            memset(return_msg, 0, MAXBROADCAST);
            sprintf(return_msg, "Client %d - Name: %s\n", p->sockfd, p->name);
            send(client->sockfd, return_msg, strlen(return_msg), 0);
        }
        return;
    } else if (strcmp("myname", cmd) == 0){
        sprintf(return_msg, "Current Username: %s\n", client->name);
    } else if (strcmp("quit", cmd) == 0){           // new else if branch for the new command
        // handle command here
        // must take some kind of action the user can perceive 
        // (otherwise they won't know the command worked)
        // Repeat this process for any more commands you want to add!
    } else {
        strcpy(return_msg, "Invalid Command. '/help' to list all available commands.\n");
    }

    send(client->sockfd, return_msg, strlen(return_msg), 0);
}
```

### Direct Messaging

Users can choose to directly message a specific user by prepending the message with "@". The logic for identifying a direct message is very similar to how we check for command messages. We just check the first character of the message and see if it is "@". 

When a message is identified as a direct message, we handle it in `handle_send()`. Instead of broadcasting a message, we find the intended recipients socket file descriptor by using `find_client_by_name()` from the `client.c` util file (handy, right!). Then, we `send()` the message to the intended user. Of course, various parsing logic is done to extract the adjusted message (excluding the "@user" part) and the username.

Note: The server crashes if the username tagged is invalid (no user by that name connected). This can be easily fixed by handling a `NULL` pointer returned by `find_client_by_name()`. This is in the past for me.

Users cannot direct message more than one person in a single message. Feel free to add this!

## Thats a wrap

That's it! This is a very simple TCP server with (some) complex logic. A test of my strength. There are still bugs and flaws, but I am happy I got this across the finish line without much outside assistance (AI).
