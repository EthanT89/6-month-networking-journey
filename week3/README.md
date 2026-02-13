# Week 3: Network Testing & Proxy Development (In Progress)

**Days 31-35 | Current: Day 33 of 35**

This week is focused on stress-testing the Treasure Hunt game under real-world network conditions. The game itself is identical to Week 2—what's changing is adding network simulation to expose architectural weaknesses.

---

## What's Here

This folder contains:
- **The Treasure Hunt game** (main.c/client.c) - Same game from Week 2, used as a test application
- **proxy_v1/** - Initial proof-of-concept transparent UDP proxy (Day 31)
- **proxy_v2/** - Production-ready proxy with latency simulation, packet loss, and statistics (Day 32-33)
- **docs/** - Planning documents and testing notes

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

### Lessons Learned

Network conditions aren't just a performance problem—they're an architectural problem. The game was built assuming perfect, instant communication. That assumption falls apart instantly with realistic network conditions.

What needs to be fixed:
1. **Client prediction** - Show movement immediately, reconcile with server later
2. **Entity interpolation** - Smooth out remote player movement
3. **Reliable events** - Critical state changes need ACKs and retransmission
4. **State reconciliation** - Clients need to gracefully handle corrections

These aren't optional features—they're fundamental requirements for networked games.

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
gcc main.c ./utils/time_custom.c ./utils/player.c ./utils/buffer_manipulation.c ./proxy_v2/utils/proxy_utils.c ./utils/treasure.c -lm -o main
./main
```

**Terminal 3+ - Join as clients:**
```bash
cd week3
gcc client.c ./utils/buffer_manipulation.c ./utils/player.c ./utils/time_custom.c ./proxy_v2/utils/proxy_utils.c ./utils/treasure.c -lm -o client
./client
```

Note: The game code needs sendto/recvfrom replaced with send_proxy/rec_proxy to route through the proxy. See proxy_v2/README.md for integration details.

---

## Files

- `main.c` - Treasure hunt server (identical to week2/main.c)
- `client.c` - Treasure hunt client (identical to week2/client.c)
- `common.h` - Shared constants and message IDs
- `utils/` - Shared utility files (player, treasure, timing, buffer manipulation)

For game details, see [../week2/README.md](../week2/README.md).

---

## What's Next

**Days 34-35 (Remaining this week):**
- Continue testing with different network conditions
- Document additional findings from proxy testing
- Refine proxy features based on real-world usage
- Final week reflection and lessons learned

**Week 4+ (Planned):**
- Client-side prediction for local player movement
- Server reconciliation when predictions diverge
- Entity interpolation for remote players
- Making the game actually playable with 100ms+ latency

Testing is revealing the problems. Solving them comes next.

---

**Status: Week 3 in progress. Proxy working. Testing ongoing. Two days remaining.**
