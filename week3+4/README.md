# Weeks 7+8: Network Testing & Reliability Systems

**Days 31-40 | Week 7 Complete, Week 8 Day 39 of 40**

Week 7 focused on stress-testing the Treasure Hunt game under real-world network conditions using a custom proxy. Week 8 implemented fundamental reliability features on top of UDP: acknowledgment packets, retransmission, latency tracking, connection handshakes, and client-side prediction.

---

## What's Here

This folder contains:
- **The Treasure Hunt game** (main.c/client.c): Enhanced version from Week 6 with reliability features and client prediction
- **proxy_v1/**: Initial proof-of-concept transparent UDP proxy (Week 7, Day 31)
- **proxy_v2/**: Production-ready proxy with latency simulation, packet loss, and statistics (Week 7, Days 32-35)
- **utils/reliable_packet.c/h**: ACK/retransmission system for critical game events (Week 8, Days 36-39)
- **docs/**: Planning documents, testing notes, and solution documentation

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

Week 7 built the testing infrastructure. Week 8 built the solutions:

**Week 7: The Testing Proxy (Days 31-35)**
- Transparent UDP proxy with configurable latency and packet loss
- Real-time statistics tracking
- See [proxy_v2/README.md](proxy_v2/README.md) for details

**Week 8: Reliability Framework & Client Prediction (Days 36-39)**

The testing revealed problems that couldn't be ignored. So I built solutions:

**1. Reliable Packet System** (Days 36-39)
- ACK/retransmission framework for critical events
- Sequence number tracking per packet
- Timeout-based retries (up to N attempts)
- Linked list structure for managing unacknowledged packets
- See `utils/reliable_packet.c` for implementation

**2. Connection Handshake** (Day 37)
- Three-way handshake: CONNECTION_REQUEST → CONNECTION_CONFIRMATION → ACK
- Prevents ghost players from failed connections
- Server only adds players after confirming they received their ID

**3. Client-Side Prediction** (Days 37-39)
- Client validates movement locally and applies immediately
- Instant response to player input (no waiting for server)
- Server sends position corrections when client state diverges
- Graceful reconciliation when predictions are wrong
- Makes game feel responsive even with 100ms+ latency

**4. Latency Tracking** (Day 36)
- Round-trip time measurement using LATENCY_CHECK packets
- Per-client latency tracking displayed in game UI
- Essential for debugging network issues

**5. Position Corrections** (Day 37)
- Server can override client position when validation fails
- Essential for detecting cheating or resolving sync issues
- Sends POSITION_CORRECTION packets only when needed

**6. Disconnection Detection** (Day 36)
- Server tracks last activity timestamp per player
- Removes inactive players after timeout
- Prevents lobby from filling with dead connections

### Lessons Learned

Network conditions aren't just a performance problem—they're an architectural problem. The game was built assuming perfect, instant communication. That assumption falls apart instantly with realistic network conditions.

**Critical insight:** Not all packets are created equal. Position updates can be lost because newer data arrives soon anyway. But connection requests, treasure collection, and disconnections are *state-changing events* that must be reliable. UDP needs selective reliability on top.

**Client prediction changes everything.** What felt sluggish at 50ms now feels instant. The client validates movement locally and applies it immediately, then accepts corrections from the authoritative server when needed. This is the difference between a playable networked game and an unplayable one.

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

- `main.c`: Treasure hunt server with reliability features and position validation
- `client.c`: Treasure hunt client with connection handshake, latency tracking, and client-side prediction
- `common.h`: Shared constants and message IDs (including ACKID, LATENCY_CHECK_ID, POSITION_CORRECTION_ID, etc.)
- `utils/`: Shared utility files
  - `player.c/h`: Player state management
  - `treasure.c/h`: Treasure system
  - `reliable_packet.c/h`: ACK/retransmission framework (NEW)
  - `time_custom.c/h`: Timing utilities
  - `buffer_manipulation.c/h`: Binary protocol helpers

For game overview, see [../week2/README.md](../week2/README.md) (Week 6 - Multiplayer Treasure Hunt).

---

## What's Next

**Week 7 Reflection:**

The proxy exposed every architectural assumption I'd made: that packets always arrive, that connections are instant, that latency doesn't matter. All false. The proxy doesn't lie.

**Week 8 Reflection:**

This week transformed from "just testing" into building fundamental networking infrastructure. The proxy revealed problems, but solving them required implementing reliability features that should have existed from the start.

Key takeaway: UDP isn't "unreliable networking". It's a blank canvas where you choose what to make reliable. Position updates? Unreliable is fine. Connection handshakes and treasure collection? Must be reliable. The challenge is building selective reliability without turning UDP into a worse version of TCP.

Client prediction changes everything. Movement that felt sluggish at 50ms now feels instant. The trick is graceful reconciliation when the server disagrees. Corrections need to happen without jarring the player.

**Day 40 (Tomorrow):**
- Planning Week 9 and beyond
- Documenting lessons learned from Weeks 7+8
- Designing next architectural improvements

**Week 9+ (Planned):**
- Entity interpolation for smooth remote player movement
- Enhanced retransmission logic (exponential backoff)
- Historical game state tracking for better corrections
- Treasure collection via reliable packets
- Further refinement of the prediction/reconciliation system

The infrastructure is built. Client prediction makes it feel good. Now comes polish and edge cases.

---

**Status: Week 7 complete. Week 8 Day 39/40 (planning tomorrow). Proxy working. Reliability framework implemented. Client prediction implemented and working.**
