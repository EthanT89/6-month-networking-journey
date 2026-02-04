# Game Server Utilities

Utility modules for the Multiplayer Movement Server (Month 2, Week 5). These are purpose-built for real-time game networking—tracking player positions, packing binary packets, and managing tick timing.

---

## What's Here

### Player Management (`player.c/h`)

Tracks all connected players and their state. Similar to the client utilities from Month 1, but with game-specific data like positions.

**Struct:**
```c
struct Player {
    unsigned int id;                    // Unique player ID
    unsigned char username[MAXUSERNAME]; // Display name
    struct sockaddr_in addr;            // Network address (for UDP responses)
    socklen_t addrlen;
    int x;                              // Current X position
    int y;                              // Current Y position
    struct Player* next;                // Linked list pointer
};

struct Players {
    int total_players;
    struct Player* head;
};
```

**Functions:**
- `add_player()` — Add a new player to the list
- `remove_player()` — Remove by ID
- `get_player_by_id()` — Lookup by player ID
- `get_player_by_name()` — Lookup by username
- `get_player_by_addr()` — Lookup by network address (most common for UDP)

**Why lookup by address?** UDP is connectionless. When a packet arrives, we don't have a socket to identify the sender—just their IP and port. So every incoming packet triggers a `get_player_by_addr()` to figure out who sent it.

---

### Buffer Manipulation (`buffer_manipulation.c/h`)

Same concept as the Month 1 version, but included here for the game server's binary protocol. Pack integers into bytes, unpack bytes into integers.

**Functions:**
- `packi16()` — Pack 16-bit integer (2 bytes)
- `packi32()` — Pack 32-bit integer (4 bytes)
- `unpacki16()` — Unpack 2 bytes to 16-bit integer
- `unpacki32()` — Unpack 4 bytes to 32-bit integer

**Packet format example:**
```
Position Update (client -> server):
[App ID: 2 bytes][Message ID: 2 bytes][X: 2 bytes][Y: 2 bytes]

Position Broadcast (server -> client):
[App ID: 2 bytes][Message ID: 2 bytes][Player1 ID: 2][X: 2][Y: 2][Player2 ID: 2][X: 2][Y: 2]...
```

Every field is packed manually. No JSON, no strings, just raw bytes. This keeps packets small and parsing fast—exactly what you want for real-time games.

---

### Timing Utilities (`time_custom.c/h`)

Game servers run on ticks. Every N milliseconds, the server collects all updates and broadcasts the current state. These utilities make tick timing easy.

**Functions:**
- `get_time_ms()` — Get current time in milliseconds (0-999, wraps every second)
- `interval_elapsed()` — Check if enough time has passed between two timestamps
- `interval_elapsed_cur()` — Check if enough time has passed since a timestamp

**Usage in the game loop:**
```c
int last_tick = get_time_ms();
int tick_interval = 50;  // 20 ticks per second

while (1) {
    // Process incoming packets...

    if (interval_elapsed_cur(last_tick, tick_interval)) {
        // Time for a tick! Broadcast all player positions
        broadcast_positions(players);
        last_tick = get_time_ms();
    }
}
```

**Note:** The timing wraps at 1000ms. The `interval_elapsed()` functions handle the wraparound correctly (if `t2 < t1`, it assumes a second boundary was crossed).

---

## How to Use

Include the headers and link the source files:

```c
#include "./utils/player.h"
#include "./utils/buffer_manipulation.h"
#include "./utils/time_custom.h"
```

```bash
gcc main.c ./utils/player.c ./utils/buffer_manipulation.c ./utils/time_custom.c -lm -o server
```

The `-lm` flag is needed for `pow()` in the timing utilities.

---

## What I Learned

Building these utilities for a game server felt different from the Month 1 work. Everything had to be faster, smaller, and more predictable:

- **Binary protocols** are intimidating at first, but once you get the hang of packing/unpacking, they're actually simpler than parsing strings
- **Tick-based loops** change how you think about server architecture—it's not request/response, it's collect/process/broadcast
- **UDP player lookup** is the most common operation, so it needs to be fast (or at least not slow)

The timing code took some tweaking to handle the millisecond wraparound correctly. Off-by-one errors in game loops are brutal—your tick rate suddenly doubles or halves and everything feels wrong.

---

## Related

This utility code supports the main game server in the parent directory. See `../README.md` for build instructions and the full packet format specification.
