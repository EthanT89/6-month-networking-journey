# Week 1 (Month 2): Multiplayer Movement Server

The first real-time game server. After spending Month 1 learning socket fundamentals, this week applied those concepts to build an actual multiplayer system with tick-based updates and binary protocols.

---

## What This Is

A UDP-based multiplayer server where multiple clients can connect, move around a shared coordinate space, and see each other's positions in real-time. No objectives, no scoring—just the foundation of synchronizing state across multiple clients at 30 updates per second.

**Key features:**
- UDP server managing multiple concurrent players
- 30Hz tick-based position broadcasts
- Player username registration
- Real-time state synchronization
- Binary network protocol (2-byte headers + packed data)
- Non-blocking terminal input for smooth controls

---

## Architecture

### Server (`main.c`)

The server runs on a fixed 30Hz tick rate (one update every ~33ms):

1. **Accept new connections**: Players send initial message with username
2. **Process movement updates**: Receive and validate position changes from clients
3. **Broadcast state**: Every tick, send all player positions to all clients
4. **Handle disconnections**: Remove players when they quit

**State management:**
- Linked list of active players (see `utils/player.c`)
- Each player tracks: ID, username, network address, x/y coordinates

### Client (`client.c`)

The client handles input and rendering:

1. **Non-blocking input**: Uses termios for immediate key detection
2. **Send updates**: Movement keys (WASD) send position updates to server
3. **Receive broadcasts**: Server sends full player list every tick
4. **Render**: Display all players and their coordinates

---

## Binary Protocol

All packets use a consistent format: 2-byte App ID (2005—my birth year!) followed by 2-byte Message ID, then message-specific data.

**Position Update (client → server):**
```
[APPID: 2 bytes][UPDATE_ID: 2 bytes][x: 2 bytes][y: 2 bytes]
```

**User Update (server → client, on connection):**
```
[APPID: 2 bytes][USERUPDATE_ID: 2 bytes][player_id: 2 bytes][username: MAXUSERNAME bytes]
```

**Position Broadcast (server → all clients, every tick):**
```
[APPID: 2 bytes][UPDATE_ID: 2 bytes]
[player1_id: 2][x: 2][y: 2]
[player2_id: 2][x: 2][y: 2]
...
```

Each player's data is 6 bytes (ID + x + y). With 10 players, a broadcast is ~64 bytes total.

---

## Build & Run

### Compile

From the `week1/` directory:

**Server:**
```bash
gcc main.c ./utils/time_custom.c ./utils/player.c ./utils/buffer_manipulation.c -lm -o main
```

**Client:**
```bash
gcc client.c ./utils/buffer_manipulation.c ./utils/player.c -o client
```

### Run

**Terminal 1 (Server):**
```bash
./main
```
Server listens on port 1210.

**Terminal 2+ (Clients):**
```bash
./client
```

Enter your username when prompted. Use WASD to move. Watch as other clients join and their positions update in real-time.

---

## Controls

- **w** - Move up
- **a** - Move left  
- **s** - Move down
- **d** - Move right
- **q** - Quit
- **c** - Enter command (blocks input temporarily)

---

## What I Learned

### Tick-based game loops are fundamental
Coming from Month 1's event-driven servers (respond when packets arrive), game servers need *proactive* updates. Even if nothing happens, the server broadcasts state every 33ms. This ensures clients stay synchronized.

### UDP requires different thinking than TCP
- No connections—identify clients by IP:port
- No delivery guarantees—must handle missing packets gracefully  
- No built-in ordering—sequence numbers needed for complex systems
- Advantage: low latency and no head-of-line blocking

### Binary protocols are more complex but efficient
JSON would've been easier, but:
- Binary is ~10x smaller (8 bytes vs 80+ bytes for position updates)
- Parsing is faster (direct memory access vs string parsing)
- Forces thinking about exact data layout

### Non-blocking input is tricky
Standard `scanf()` blocks the entire program. Using termios to read single keypresses without blocking was new territory. Required learning about terminal modes and immediate character input.

---

## Known Limitations

- **No client prediction**: Movement feels laggy if you have network delay
- **No interpolation**: Remote players "teleport" between positions
- **Basic collision**: No boundaries or player collision detection
- **State explosion**: Broadcasting full state every tick doesn't scale past ~20 players
- **Localhost only**: Haven't tested over actual network conditions

These limitations became the focus of Week 2 (adding gameplay) and Week 3 (testing with network simulation).

---

## Files & Utilities

- `main.c` - Server implementation (538 lines)
- `client.c` - Client implementation (726 lines)
- `common.h` - Shared constants and message IDs
- `utils/player.c/h` - Player management (linked list operations)
- `utils/buffer_manipulation.c/h` - Binary packing/unpacking functions
- `utils/time_custom.c/h` - Timing utilities for tick management

See [utils/README.md](utils/README.md) for detailed utility documentation.

---

## What's Next

[Week 2](../week2/) transforms this movement server into an actual game—adding treasures, scoring, collision detection, and a dynamic viewport. The core networking stays the same; we're just adding gameplay on top.

---

**Status: Week 1 complete. Real-time multiplayer synchronization works. Ready to build an actual game.**