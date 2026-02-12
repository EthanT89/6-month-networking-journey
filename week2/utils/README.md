# Game Server Utilities (Week 2 & 3)

Utility modules for the Multiplayer Treasure Hunt game. These extend the Week 1 utilities with treasure management and are shared between Week 2 (gameplay implementation) and Week 3 (network testing).

---

## What's Here

### Player Management (`player.c/h`)

Tracks all connected players and their game state. Extends the basic player tracking from Week 1 with treasure hunt-specific data.

**Struct:**
```c
struct Player {
    unsigned int id;                    // Unique player ID
    unsigned char username[MAXUSERNAME]; // Display name
    struct sockaddr_in addr;            // Network address (for UDP routing)
    socklen_t addrlen;
    int x;                              // Current X position
    int y;                              // Current Y position
    int score;                          // Points accumulated from treasures
    struct Player* next;                // Linked list pointer
};

struct Players {
    int total_players;
    struct Player* head;
};
```

**Functions:**
- `add_player()` — Add new player to the game
- `remove_player()` — Remove by ID (handles memory cleanup)
- `get_player_by_id()` — Lookup by player ID
- `get_player_by_name()` — Lookup by username
- `get_player_by_addr()` — Lookup by network address (primary for UDP)

**UDP addressing:** Since UDP is connectionless, incoming packets only have a source IP:port. The server uses `get_player_by_addr()` to match packets to players.

---

### Treasure Management (`treasure.c/h`)

Similar linked-list structure to players, but for managing randomly-spawned treasures across the map.

**Struct:**
```c
struct Treasure {
    unsigned int id;                    // Unique treasure ID
    int x;                              // X coordinate
    int y;                              // Y coordinate
    int value;                          // Point value (1-3)
    struct Treasure* next;
};

struct Treasures {
    int total_treasures;
    struct Treasure* head;
};
```

**Functions:**
- `new_treasure()` — Allocate and initialize a treasure
- `add_treasure()` — Add to the linked list
- `remove_treasure()` — Remove by ID
- `get_treasure_by_id()` — Lookup by treasure ID
- `get_treasure_by_coords()` — Check if treasure exists at (x,y)
- `print_treasures()` — Debug helper

**Collision detection:** The server uses `get_treasure_by_coords()` every time a player moves to check if they've collected a treasure.

**State synchronization:** When a treasure is collected:
1. Server broadcasts DELTREASURE message to all clients
2. Server spawns new treasure at random location
3. Server broadcasts NEWTREASURE message to all clients

---

### Buffer Manipulation (`buffer_manipulation.c/h`)

Binary packing/unpacking for the game's network protocol. Same concept as Week 1, but included here for the treasure hunt's extended message types.

**Functions:**
- `packi16()` — Pack 16-bit integer into 2 bytes
- `packi32()` — Pack 32-bit integer into 4 bytes
- `unpacki16()` — Unpack 2 bytes to 16-bit integer
- `unpacki32()` — Unpack 4 bytes to 32-bit integer

**Typical usage:**
```c
// Packing a treasure spawn message
unsigned char packet[MAXBUFSIZE];
int offset = 0;

packi16(packet + offset, APPID); offset += 2;
packi16(packet + offset, NEWTREASURE_ID); offset += 2;
packi16(packet + offset, treasure->id); offset += 2;
packi16(packet + offset, treasure->x); offset += 2;
packi16(packet + offset, treasure->y); offset += 2;
packi16(packet + offset, treasure->value); offset += 2;

// Send 12-byte packet
sendto(sockfd, packet, offset, 0, &client_addr, addr_len);
```

**Network byte order:** These functions handle endianness correctly, ensuring consistent byte order across different architectures.

---

### Timing Utilities (`time_custom.c/h`)

Millisecond-precision timing for tick-based game loops.

**Functions:**
- `get_time_ms()` — Current time in milliseconds (0-999, wraps at 1000)
- `interval_elapsed()` — Check if interval passed between two timestamps
- `interval_elapsed_cur()` — Check if interval passed since timestamp

**Game loop pattern:**
```c
int last_broadcast = get_time_ms();
int tick_interval = 33;  // 30 ticks per second

while (1) {
    // Process incoming packets (non-blocking)
    
    if (interval_elapsed_cur(last_broadcast, tick_interval)) {
        // Time for a tick!
        broadcast_all_positions(players);
        broadcast_all_treasures(treasures);
        last_broadcast = get_time_ms();
    }
}
```

**Wraparound handling:** Since `get_time_ms()` wraps every second, the `interval_elapsed()` functions detect when `current_time < start_time` and handle it correctly.

---

## Why Linked Lists?

Both players and treasures use linked lists instead of arrays because:

1. **Dynamic size**: Players connect/disconnect constantly. Treasures spawn/despawn frequently. Arrays require fixed sizes or expensive resizing.

2. **Fast add/remove**: O(1) to add to head or remove by pointer (when you have it).

3. **Simple memory management**: Just malloc/free individual nodes. No shifting elements around.

**Tradeoff:** Lookup is O(n), but with ~10 players and ~80 treasures, this is fast enough. If scaling to 1000+ entities, a hash map or spatial partitioning would be better.

---

## Using These Utilities

Include the headers in your game code:

```c
#include "./utils/player.h"
#include "./utils/treasure.h"
#include "./utils/buffer_manipulation.h"
#include "./utils/time_custom.h"
```

Link when compiling:
```bash
gcc main.c ./utils/player.c ./utils/treasure.c ./utils/buffer_manipulation.c ./utils/time_custom.c -lm -o main
```

(The `-lm` flag links the math library, needed for `sqrt()` in distance calculations.)

---

## Differences from Week 1 Utilities

Week 1 utilities (`week1/utils/`) are similar but don't include:
- Treasure management (obviously—no treasures in Week 1)
- Score tracking in Player struct
- Some helper functions specific to the treasure hunt game

Week 2/3 utilities are a superset—they can do everything Week 1's do, plus game-specific features.

---

**These utilities power both Week 2's gameplay implementation and Week 3's network stress testing.**
