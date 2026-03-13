# Week 5: Simple Multiplayer Movement Server

## Goal of this doc

The goal of this document is to better understand what data structures will be necessary to account for variable amounts of players, as well as a general idea of what functions and utilities may be needed.

Will it be necessary to access by index? By node? Linked list? 

## Requirement Breakdown

We need to store several data points about players:
1. Player coordinates (x,y)
2. Player ID (possibly username later)
3. Socket file descriptor (for communication)
4. Anything else?

Thus, we need a data structure capable of storing multiple attributes, optimally with the possibility of expanding them. A structure to store these clients will also be needed. We will appropriately call the data structures `Player`, and `Players`, respectively.

We will regularly need to update player coordinates. However, sometimes there will be no update necessary (ie. if a player does not move). This means we don't need to access a particular player's data every loop. Knowing this, it feels inefficient to use a linked list.

So, we need to be able to efficiently loop through data (for sending and updating), while also having the ability to efficiently 'skip' a user if no update is necessary.

Function-wise, the following will be necessary to fulfill the above functionality:
- `update_player_state()` -- Apply updates (post filtering) to the corresponding users, if any.
- `verify_action()` -- Verify an event's legitimacy via cross checking server data. This currently does not serve a strong purpose, considering we only take inputs, but is a good baseline security check to practice.
- `broadcast_state()` -- The first step of broadcasting state to players. This will build the packet and deliver it via `broadcast()`. The justification for having a separate `broadcast()` function lies in the opportunity to send more data than just updates (ie. welcome messages, server logs, etc.)
- `broadcast()` -- Given a message, send it to all connections.
- `handle_user_data()` -- When a socket (connection) is determined to be ready to read, this function will read its data and begin parsing for info. If valid state update, calls `verify_action()` then `update_player_state()`

A utility file for managing users (`player.c`) will be necessary to create and manage the `player` data structures. This includes adding, removing, etc.

Player updates will contain the following:
1. Player input (up, down, left, right)
2. timestamp
3. user id
4. sequence number

We will need a way to unpack these messages and fill a data structure with them. Something like `client_update`.

And the following helper functions (which may be extracted to their own dedicated helper files):
- `unpack_update()` 
- `pack_update()`

These names sound very similar, and are indicative that the same data is being packed and unpacked. However, different data is being packed than is being unpacked, so for clarification these names should be reconsidered.



## Server Init

`get_listener()` -- creates and returns the server's listening socket, which will wait for connections
`accept_connection()` -- accepts a connection from the listener socket
`add_to_pfds()` -- add a sockfd to the pfds