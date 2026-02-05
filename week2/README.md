# Multiplayer Treasure Hunt

Week 2 builds on the foundation from Week 1, transforming the basic multiplayer movement server into an actual game with objectives, scoring, and dynamic gameplay. Players now compete to collect randomly-spawned treasures across a 100x100 map, with real-time position updates and a moving viewport that follows the player.

## What Makes This Cool

This is a fully functional multiplayer treasure hunting game built entirely from scratch using raw UDP sockets and binary packet encoding. No game engines, no frameworks - just C and networking fundamentals.

**New features added this week:**
- **Treasure system**: Randomly spawning treasures (up to 80 concurrent) with varying point values (1-3 pts)
- **Scoring system**: Players accumulate points by reaching treasure locations before others
- **Dynamic viewport**: A 10x10 viewport that follows the player's position on a 100x100 map
- **Collision detection**: Server validates all movements against boundaries and other players
- **Bot mode**: Type "bot" as your username to watch an AI player pathfind to the nearest treasure
- **Connection limits**: Server enforces a maximum of 12 concurrent players
- **Smart treasure management**: Collected treasures are broadcast as removed, new ones spawn immediately

## The Journey (Challenges & Solutions)

### The Segfault Saga
Early on, I hit a nasty segfault that took way too long to track down. The issue? I was trying to print treasure coordinates before any treasures had been received from the server. The tricky part was that my debug `printf` statements weren't showing up because stdout buffering meant they never flushed before the crash. Lesson learned: always guard against NULL pointers, especially during startup, and use `fflush(stdout)` or stderr for critical debug messages.

### Treasure State Management
One of the most interesting challenges was managing treasure state across the server and multiple clients. The server needed to:
1. Generate random treasure locations without overlapping players
2. Broadcast new treasures to all connected clients
3. Detect when a player reaches a treasure
4. Notify all clients to remove the collected treasure
5. Immediately spawn a replacement

I solved this by creating a `treasure.c` utility with a linked-list structure (similar to `player.c`) that supports add/remove by ID and coordinate-based lookups. The server maintains the authoritative state and broadcasts NEWTREASURE_ID and DELTREASURE_ID messages to keep clients synchronized.

### The Moving Viewport Problem
Initially, I had a static viewport that showed coordinates 0-9. But on a 100x100 map, this was useless once you moved away from the origin. The solution was to calculate viewport bounds dynamically based on player position:
- If player is near the edge (< 5 units from boundary), clamp the viewport to the map edge
- Otherwise, center the viewport on the player
- This creates a smooth "camera follows player" effect in the terminal

The viewport rendering also includes alternating `,` and `.` symbols on a checkerboard pattern to create an illusion of movement even on static tiles.

### Bot Mode Intelligence
The bot uses a simple but effective pathfinding algorithm:
1. Find the nearest treasure using Euclidean distance (`sqrt(dx² + dy²)`)
2. Randomly choose between moving toward the treasure on the X-axis or Y-axis (adds variety)
3. Move at randomized intervals (50-500ms) to create more natural behavior
4. If multiple bots are running, they race to treasures organically

## Architecture

The codebase is organized into modular utilities:

```
week2/
├── main.c              (538 lines) - Game server logic
├── client.c            (726 lines) - Client rendering and input handling  
├── common.h            - Shared constants and message IDs
├── GameDesign.md       - Original design document
└── utils/
    ├── player.c/h      - Player management (linked list, add/remove/lookup)
    ├── treasure.c/h    - Treasure management (same pattern as players)
    ├── time_custom.c/h - Timing utilities for tick-based updates
    └── buffer_manipulation.c/h - Binary packing/unpacking (packi16/unpacki16)
```

**Total: ~1,628 lines of C code**

## Game State Structure

**Server maintains:**
```c
struct State {
    int sockfd;
    int player_id_ct;       // Auto-incrementing player IDs
    int treasure_id_ct;     // Auto-incrementing treasure IDs
    struct Players *players;
    struct Treasures *treasures;
};
```

**Client maintains:**
```c
struct User {
    int x, y;               // Player position
    int score;              // Accumulated points
    char name[MAXUSERNAME];
    struct Treasures *treasures;  // Local copy of all treasures
};
```

## Packet Protocol

All packets start with a 2-byte App ID (2005 - the year I was born!) followed by a 2-byte Message ID.

**Position Update (server → all clients, every 33ms):**
```
[APPID:2][UPDATE_ID:2][id:2][x:2][y:2][score:2] ... (repeated per player)
```

**New Treasure (server → all clients):**
```
[APPID:2][NEWTREASURE_ID:2][id:2][x:2][y:2][value:2]
```

**Delete Treasure (server → all clients):**
```
[APPID:2][DELTREASURE_ID:2][id:2]
```

**User Update (server → all clients when player joins/renames):**
```
[APPID:2][USERUPDATE_ID:2][id:2][username:MAXUSERNAME]
```

**Connection Rejected (server → client when full):**
```
[APPID:2][REJECT_CONNECTION_ID:2]
```

## Build & Run

### Prerequisites
- GCC compiler
- Linux/Unix environment (uses termios for non-blocking input)
- No external dependencies

### Compile

From the `week2/` directory:

**Server:**
```bash
gcc main.c ./utils/time_custom.c ./utils/player.c ./utils/buffer_manipulation.c ./utils/treasure.c -lm -o main
```

**Client:**
```bash
gcc client.c ./utils/buffer_manipulation.c ./utils/player.c ./utils/time_custom.c ./utils/treasure.c -lm -o client
```

### Run

**Terminal 1 (Server):**
```bash
./main
```
The server will start listening on port 1209 (my birthday!) and spawn 80 initial treasures.

**Terminal 2+ (Clients):**
```bash
./client
```
Enter a username when prompted. Run multiple clients in separate terminals to play with friends (or against bots).

**Terminal N (Bot):**
```bash
./client
# When prompted, type: bot
```
Watch the bot pathfind to treasures automatically.

## Controls (Client)

- `w` / `a` / `s` / `d` - Move up / left / down / right
- `q` - Quit
- `c` - Enter command mode (blocking input)

### Commands
- `/help` - Show available commands
- `/updatename` - Change your username
- `/reset` - Reset your position to (0,0)
- `/quit` - Exit gracefully

## Customization

Want to tweak the game? Edit `common.h`:

```c
#define TICKRATE 33          // Server update rate (ms) - lower = faster updates
#define BOUNDX 100           // Map width
#define BOUNDY 100           // Map height
#define MAXTREASUREVAL 3     // Max points per treasure
#define MAXTREASURES 80      // Max concurrent treasures
#define MAXPLAYERS 12        // Max connected players
#define MYPORT "1209"        // Server port
```

After editing, recompile both server and client.

**Example modifications:**
- Make a tiny 20x20 arena: `BOUNDX 20`, `BOUNDY 20`
- High-stakes gameplay: `MAXTREASUREVAL 10`, `MAXTREASURES 5`
- Crowded server: `MAXPLAYERS 50` (if your network can handle it)
- Ultra-responsive: `TICKRATE 16` (60 FPS updates)

## Viewport Example

```
Nearest Treasure: (1,4) - 2 points
________________________________________
|  Player     |   Coords    |  Score   |
|_____________|_____________|__________|
|  ethan      |  (+55,+70)  |  14 pts  |
|_____________|_____________|__________|
|  bot        |  (+58,+68)  |  32 pts  |
|  james      |  (+12,+45)  |  08 pts  |
|_____________|_____________|__________|

X .   ,   .   ,   . 
X                   
X ,   ,   , $ ,   , 
X   e                
X .   ,   .   ,   . 
X $                 
X ,   , o ,   e   , 
X                   
X .   ,   .   ,   . 
X X X X X X X X X X 
```

**Legend:**
- `o` - You
- `e` - Other players
- `$` - Treasure
- `X` - Boundary wall
- `,` / `.` - Empty space (alternating pattern)

## What I Learned

1. **State synchronization is hard**: Keeping treasure state consistent across server and multiple clients required careful message ordering and ID management.

2. **NULL checks everywhere**: The segfault taught me to always validate pointers before dereferencing, especially during initialization phases.

3. **Modular design pays off**: Reusing the linked-list pattern from `player.c` for `treasure.c` made implementation much faster.

4. **Binary protocols are efficient but fragile**: Every byte matters. Off-by-one errors in offsets cause silent data corruption that's hard to debug.

5. **Terminal rendering limitations**: Creating a "game" feel in a terminal is challenging but rewarding. The alternating `,` and `.` pattern was a creative solution to make static text feel dynamic.

6. **Testing with bots is essential**: Bot mode made it easy to test multi-client scenarios without needing multiple people.

## Known Limitations

- **No persistence**: Game state resets when the server restarts
- **UDP reliability**: Packets can be lost (though it's rare on localhost)
- **Terminal-only**: No GUI, all rendering is ASCII in the terminal
- **No win condition**: Scoring tracks but game never "ends"
- **Collision detection is server-side only**: Client displays may briefly show invalid moves before correction

## Next Steps (Week 3?)

Some ideas for future enhancements:
- TCP implementation for reliability
- Persistent leaderboard (file or database)
- Power-ups (speed boost, treasure radar)
- Team modes
- Obstacles/rocks/walls that block movement
- Win condition and game reset
- Ncurses for smoother rendering

## Running from GitHub

```bash
git clone https://github.com/EthanT89/6-month-networking-journey.git
cd 6-month-networking-journey/week2
# Follow build instructions above
```

## Credits

Built as part of my 6-month networking learning journey. This project explores UDP networking, binary protocols, game server architecture, and terminal-based UI design - all from scratch in C.

---

*"The best way to learn networking is to build something that breaks in interesting ways." - me, after debugging stdout buffering for an hour*