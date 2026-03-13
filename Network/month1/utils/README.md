# Month 1 Utilities

Shared utility modules used across the Month 1 projects. As I built more complex servers, I kept finding myself rewriting the same code. So I pulled common patterns into these reusable files.

---

## What's Here

### Buffer Manipulation (`buffer_manipulation.c/h`)

The workhorse of binary network protocols. These functions pack and unpack integers into byte buffers for transmission over the network.

**Functions:**
- `packi16()` — Pack a 16-bit integer into 2 bytes
- `packi32()` — Pack a 32-bit integer into 4 bytes
- `packi64()` — Pack a 64-bit integer into 8 bytes
- `unpacki16()` — Unpack 2 bytes into a 16-bit integer
- `unpacki32()` — Unpack 4 bytes into a 32-bit integer
- `append_buf_after_sequence()` — Append a message after a 4-byte header

**Why this matters:** Network protocols need to send structured data. You can't just send a C struct over the wire—different machines have different byte orders and padding. Packing integers manually ensures consistent serialization.

**Example:**
```c
unsigned char buf[32];
unsigned int player_id = 42;
unsigned int x_pos = 100;

packi16(buf, player_id);
packi16(buf + 2, x_pos);

// Now buf contains: [0x00, 0x2A, 0x00, 0x64]
// Ready to send over the network!
```

---

### ACK Bitmap (`ack.c/h`)

The reliability layer for my UDP protocol experiments. TCP gives you reliable delivery for free—UDP doesn't. This module implements acknowledgment tracking using a sliding window bitmap.

**Key concepts:**
- 32-bit bitmap where each bit represents a packet's ACK status
- Bit 0 = most recent packet, Bit 31 = 31 packets ago
- Automatic retransmission after 1 second timeout

**Functions:**
- `update_outgoing_acks()` — Shift the bitmap when new ACKs arrive
- `check_ack_membership()` — Check if a packet ID has been ACK'd
- `unpack_acks()` — Convert bitmap bytes to a list of bits
- `convert_bits_to_id()` — Convert bit positions to packet IDs
- `construct_ack_packet()` — Build an ACK packet for transmission
- `add_pending_packet()` — Track sent packets awaiting ACK
- `get_seconds()` — High-precision timing for retransmission

**The hard part:** Bit manipulation in C is tricky. Shifting bytes, handling carry bits, keeping track of MSB vs LSB ordering... Week 3 was humbling. This code works, but I learned a lot about planning before coding.

---

### Client Management (`client.c/h`)

A simple linked list for tracking connected clients. Used in the TCP Chat Server to manage usernames and direct messaging.

**Struct:**
```c
struct Client {
    int sockfd;                      // Socket file descriptor
    unsigned char name[MAXCLIENTNAME]; // Username
    struct Client *next;
    struct Client *prev;
};

struct LLClients {
    struct Client *head;
    int n;  // Number of clients
};
```

**Functions:**
- `create_client()` — Allocate and initialize a new client
- `add_client()` — Add to the linked list
- `remove_client()` — Remove by socket fd (handles all edge cases)
- `find_client_by_sockfd()` — Lookup by socket
- `find_client_by_name()` — Lookup by username (for direct messaging)
- `print_clients()` — Debug helper

**Why a linked list?** Clients connect and disconnect constantly. Arrays require shifting or tracking holes. Linked lists make add/remove O(1). The tradeoff is O(n) lookup, but with a few dozen clients, it doesn't matter.

---

### Poll File Descriptors (`pfds.c/h`)

Helper functions for managing the `struct pollfd` array used with `poll()`. The array grows dynamically as clients connect.

**Functions:**
- `add_to_pfds()` — Add a new file descriptor (doubles array size if full)
- `remove_from_pfds()` — Remove by swapping with the last element

**Note:** These functions are separate from the Client utilities because `poll()` needs its own array of `pollfd` structs. Yes, it's a bit redundant to have two data structures tracking clients, but they serve different purposes: `pfds` is for I/O multiplexing, `clients` is for application-level data like usernames.

---

## How to Use

Include the header in your source file and link the `.c` file when compiling:

```c
#include "../utils/buffer_manipulation.h"
#include "../utils/client.h"
```

```bash
gcc your_server.c ../utils/buffer_manipulation.c ../utils/client.c -o server
```

---

## What I Learned

Building these utilities taught me a lot about C:
- Pointer arithmetic (buffer packing is all about `buf + offset`)
- Memory management (who owns the memory? who frees it?)
- Bit manipulation (shifting, masking, the works)
- The value of clean interfaces (once these worked, I could forget about the details)

The ACK bitmap was the hardest. It took me most of Week 3 to get right, and even then it's not complete. But struggling through it taught me more about how TCP works under the hood than any tutorial could.
