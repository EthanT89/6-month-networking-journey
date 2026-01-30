# Multi-Client Chat Server Planning

## Overview

### Goal:
This application allows multiple clients, across networks, to connect to a single chat server and communicate with one another. User's should be able to send and receive chat logs, and seamlessly connect/disconnect from the server.

### Nice-to-have's:
Users can utilize "direct messaging". Without having to leave or join a new chat server, user's can view and directly message other users in the server. These messages are only broadcasted to the desired user and not to other users.

Users can create usernames upon joining the server.

## Plan

### Broad Setup
A singular server will handle all connected clients. This server will handle all of the server-side logic, outsourcing universal logic and code to a unified utility file. The client servers will also utilize this utility file.

A client server will connect to the chat server and receive/send messages to and fro the server. The client server is also responsible for packing/unpacking data before using it. This server can disconnect from the main server at any moment.

A shared utility file will be created to manage shared functionality. Draw examples from `buffer_manipulation.c` and `ack.c` in `./utils`. These files may be used and can be expanded. `ack.c` will remain unused due to TCP having built-in ACK systems.

Shared logic includes, but is not limited to:
- Packing/unpacking data.
- Common Global Variables (APPID, MAXBUFSIZE, etc.)

### File Specific Setups (Server)
The server will first setup the socket and bind to a port. This is fairly straightforward and will not be discussed further.

The server will use `poll()` to manage multiple connections. Thus, we will need a data structure to store and manage `sockfd`'s. It will be optimal to use a linked list so that we can easily remove/insert clients from this structure. This will make the management of dropping clients easier.

We will also need a pollfd structure to manage the socket file descriptors. There will be two data structures for the users: An extensive linked list structure of connected clients, used for managing messages, usernames, and most functionality, and a compact list of socket file descriptors used for `poll()`'ing. 

For each client, we will need a username, file descriptor, and status. We do not need address because the socket file descriptor handles that (thanks TCP). 

Each packet will contain a `protocol_id` which ensures packets are only recieved/sent by connected members. This marks the start of a packet. The next part of the packet will just be the message. Simple enough. Username is stored by the first message the client sends, and timestamps/addr/sequencing is all handled already.

Each message will need to be parsed if we are to implement the direct messaging. Checking for "@user", and calling a user-specific broadcast system.

We will need as system for tracking all connections (using `poll()`), handling new connections and disconnections, and broadcasting messages.
 
### File Specific Setups (Client)

### File Specific Setups (Shared Utils)







### Server Structure

Sockfd's for poll() -- Linked list
