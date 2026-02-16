/*
 * common.h -- common file for constants, small structs, and symbols
 */

#ifndef COMMON_H
#define COMMON_H

// Global Constants
#define TICKRATE 33 // time between each tick in milliseconds
#define MYPORT "1209" // arbitrary port number (my birthday)
#define MAXBACKLOG 10 // max clients waiting to connect
#define MAXUSERNAME 15 // max length of a player username
#define MAXBUFSIZE 1000 // max size of a buffer (packet)
#define MAXCOMMANDSIZE 15 // max size of a command message
#define STARTING_OFFSET 4 // standard offset for packets (4 bytes for APPID and MSGTYPE)
#define BOUNDX 100 // Size of map in the X plane
#define BOUNDY 100 // Size of map in the Y plane
#define MAXTREASUREVAL 3 // max value of a single treasure
#define MAXTREASURES 80 // max instances of treasures in a single instant
#define MAXPLAYERS 12 // max concurrently connected players on a single instance of the server

// Various ids that identify a certain message type
#define USERUPDATE_ID 6123 // an update on users. For a client, this means other players, for the server, an updated username, id, etc.
#define APPID 2005 // ID for the app (the year i was born!)
#define COMMAND_ID 8008 // A command update (/help, /reset, etc)
#define UPDATE_ID 2026 // Positional updates
#define EXIT_ID 2581 // User disconnection
#define NEWTREASURE_ID 450 // new treasure update
#define DELTREASURE_ID 451 // delete treasure update
#define ERROR_ID 666 // some kind of error has occurred
#define REJECT_CONNECTION_ID 667 // reject an attempted connection to the server (often from being full)
#define YOUR_ID_IS 1001 // User ID update
#define POSITION_CORRECTION_ID 1002 // positional client correction
#define LATENCY_CHECK_ID 1003 // round trip packet used to calculate latency

// Viewport Symbols
#define BOUNDARY_SYMBOL "X" // Represents a boundary on the map
#define PLAYER_SYMBOL "o" // Represents the current player
#define PLAYER_SYMBOL2 "e" // Represents any other players
#define EMPTY_SYMBOL " " // Represents an empty tile (tangent with '.' and ',')
#define TREASURE_SYMBOL "$" // Represents a treasure

// Commands INT form
#define UPDATE_USERNAME 55 // request to update a player's username
#define RESET_COORDS 56 // request to reset a player's coordinates

struct Network {
    int latency_ms;
    int last_tick_sent;
};

#endif