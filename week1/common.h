#ifndef COMMON_H
#define COMMON_H

// Global Constants
#define TICKRATE 33 // time between each tick in milliseconds
#define MYPORT "1209" // arbitrary port number (my birthday)
#define MAXBACKLOG 10 // max clients waiting to connect
#define MAXUSERNAME 15 // max length of a player username
#define MAXBUFSIZE 100 // max size of a buffer (packet)
#define MAXCOMMANDSIZE 15 // max size of a command message
#define STARTING_OFFSET 4 // standard offset for packets (4 bytes for APPID and MSGTYPE)

// Various ids that identify a certain message type
#define USERUPDATE_ID 6123 // an update on users. For a client, this means other players, for the server, an updated username, id, etc.
#define APPID 2005 // ID for the app (the year i was born!)
#define COMMAND_ID 8008 // A command update (/help, /reset, etc)
#define UPDATE_ID 2026 // Positional updates
#define EXIT_ID 2581 // User disconnection

// Commands INT form
#define UPDATE_USERNAME 55 // request to update a player's username
#define RESET_COORDS 56 // request to reset a player's coordinates

#endif