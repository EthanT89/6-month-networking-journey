/*
 * common.h -- A common file for structs/functions/variables shared by TCP server and client files
 */

#ifndef _COMMON_H
#define _COMMON_H


#define MYPORT "1209" // Port number that the server will run on.
#define MAXCLIENTNAME 10 // Max characters allowed to store a client name 
#define MAXBUFSIZE 200 // Max receiving buf size

// Max broadcast message size. Accounts for extra characters prepending the message content.
// eg "hello" received vs "user: hello" broadcasted
#define MAXBROADCAST MAXBUFSIZE + MAXCLIENTNAME + 5
#define NAMENOTSET "hk35vkdf5m" // arbitrary string that indicates a name is not set. username defaults to this

#endif