# Week 3: Network Testing & Reliability Systems

**Days 31-35 | Complete**

This week started with stress-testing the Treasure Hunt game under real-world network conditions using a custom proxy. The testing exposed critical weaknesses, which led to implementing fundamental reliability features on top of UDP: acknowledgment packets, retransmission, latency tracking, and connection handshakes.

---

## What's Here

This folder contains:
- **The Treasure Hunt game** (main.c/client.c) - Enhanced version from Week 2 with reliability features
- **proxy_v1/** - Initial proof-of-concept transparent UDP proxy (Day 31)
- **proxy_v2/** - Production-ready proxy with latency simulation, packet loss, and statistics (Day 32-35)
- **utils/reliable_packet.c/h** - ACK/retransmission system for critical game events
- **docs/** - Planning documents, testing notes, and solution documentation

---

## The Testing Journey

After building the treasure hunt game in Week 2, everything worked perfectly on localhost. But localhost is a lie. Real networks have latency, packet loss, and congestion. I needed to see how the game behaved under realistic conditions.

So I built a UDP proxy that could simulate:
- Variable latency (50ms to 500ms+)
- Packet loss (5% to 30%)
- Real-time statistics tracking

### What I Discovered

**With 50ms latency:**
- Movement felt sluggish
- Treasure collection had noticeable delay
- Still playable, but frustrating

**With 100ms+ latency:**
- Nearly unplayable without client prediction
- Players would "jump" to their actual position after perceiving they'd moved
- Racing for treasures became a guessing game

**With packet loss:**
- Game state broke completely
- Missing player updates caused rendering bugs
- Critical events (connections, treasure spawns) failed silently
- Treasures would appear collected on server but remain on client
- Players would disconnect without the server detecting it

### What I Built

The testing revealed problems that couldn't be ignored. So I built solutions:

**1. Reliable Packet System** (Days 33-35)
- ACK/retransmission framework for critical events
- Sequence number tracking per packet
- Timeout-based retries (up to N attempts)
- Linked list structure for managing unacknowledged packets
- See `utils/reliable_packet.c` for implementation

**2. Connection Handshake** (Day 34)
- Three-way handshake: CONNECTION_REQUEST → CONNECTION_CONFIRMATION → ACK
- Prevents ghost players from failed connections
- Server only adds players after confirming they received their ID

**3. Latency Tracking** (Day 32-33)
- Round-trip time measurement using LATENCY_CHECK packets
- Players track their own latency to server
- Helps with debugging and future client prediction work

**4. Position Corrections** (Day 33)
- Server can override client position when validation fails
- Essential for detecting cheating or resolving sync issues
- Sends POSITION_CORRECTION packets only when needed

**5. Disconnection Detection** (Day 32)
- Server tracks last activity timestamp per player
- Removes inactive players after timeout
- Prevents lobby from filling with dead connections

### Lessons Learned

Network conditions aren't just a performance problem—they're an architectural problem. The game was built assuming perfect, instant communication. That assumption falls apart instantly with realistic network conditions.

**Critical insight:** Not all packets are created equal. Position updates can be lost—newer data arrives soon anyway. But connection requests, treasure collection, and disconnections are *state-changing events* that must be reliable. UDP needs selective reliability on top.

What still needs work:
1. **Better retransmission logic** - Current system is basic; needs exponential backoff
2. **State reconciliation** - Clients need to gracefully handle corrections without jarring jumps

Testing revealed the problems. Building reliability features solved the critical ones. Client-side prediction is the next frontier.

---

## The Proxy

See [proxy_v2/README.md](proxy_v2/README.md) for the full story of how the proxy works.

**Quick summary:**
- Transparent wrapper functions (send_proxy/rec_proxy) replace sendto/recvfrom
- Integration is literally find-and-replace
- Simulates latency and packet loss
- Tracks real-time statistics
- Works with any UDP application

---

## Running the Tests

**Terminal 1 - Start the proxy:**
```bash
cd proxy_v2
gcc proxy.c ./utils/proxy_utils.c ../utils/time_custom.c ./utils/delayed_packet.c -o proxy
./proxy 100 5  # 100ms latency, 5% packet loss
```

**Terminal 2 - Start the game server:**
```bash
cd week3
gcc main.c ./utils/time_custom.c ./utils/player.c ./utils/buffer_manipulation.c ./proxy_v2/utils/proxy_utils.c ./utils/treasure.c ./utils/reliable_packet.c -lm -o main
./main
```

**Terminal 3+ - Join as clients:**
```bash
cd week3
gcc client.c ./utils/buffer_manipulation.c ./utils/player.c ./utils/time_custom.c ./proxy_v2/utils/proxy_utils.c ./utils/treasure.c ./utils/reliable_packet.c -lm -o client
./client
```

Note: The game code needs sendto/recvfrom replaced with send_proxy/rec_proxy to route through the proxy. See proxy_v2/README.md for integration details.

---

## Files

- `main.c` - Treasure hunt server with reliability features
- `client.c` - Treasure hunt client with connection handshake and latency tracking
- `common.h` - Shared constants and message IDs (including ACKID, LATENCY_CHECK_ID, etc.)
- `utils/` - Shared utility files:
  - `player.c/h` - Player state management
  - `treasure.c/h` - Treasure system
  - `reliable_packet.c/h` - ACK/retransmission framework (NEW)
  - `time_custom.c/h` - Timing utilities
  - `buffer_manipulation.c/h` - Binary protocol helpers

For game overview, see [../week2/README.md](../week2/README.md).

---

## What's Next

**Week 3 Reflection:**

This week transformed from "just testing" into building fundamental networking infrastructure. The proxy revealed problems, but solving them required implementing reliability features that should have existed from the start.

Key takeaway: UDP isn't "unreliable networking"—it's a blank canvas. You choose what to make reliable. Position updates? Unreliable is fine. Connection handshakes and treasure collection? Must be reliable. The challenge is building selective reliability without turning UDP into a worse version of TCP.

**Week 4+ (Planned):**
- Client-side prediction for local player movement
- Server reconciliation when predictions diverge  
- Enhanced retransmission logic (exponential backoff)
- Historical game state tracking for better corrections
- Treasure collection via reliable packets
- Making the game actually playable at 100ms+ latency

The infrastructure is built. Now comes the hard part: making it feel good.

---

**Status: Week 3 complete. Proxy working. Reliability framework implemented. Ready for client prediction.**
