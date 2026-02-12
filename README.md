# Real-Time Systems From Scratch

**A 6-month deep work project to master real-time networked systems in C.**

I'm learning how multiplayer games actually work by building networking systems from the ground up—no frameworks, no game engines, just C and the fundamentals.

20 years old. Computer Science and Engineering student at UC Irvine. Software Engineer at First American.

---

## What This Is

This repository documents everything I've built during my 6-month commitment to learning real-time networked systems. Every weekday at 2pm, I sit down for 2.5 hours of focused work. No phone. No distractions. Just building.

The goal isn't just to learn networking concepts—it's to build the discipline to show up consistently and tackle hard problems. I'm documenting the entire journey through code, architecture notes, and reflections on what worked and what didn't.

---

## The Journey So Far

### Month 1: Fundamentals (January 2025)
**Status: Complete. 20 days straight.**

**Week 1: TCP Basics**
- Worked through Beej's Guide to Network Programming
- Built basic TCP echo server and client
- Learned socket programming fundamentals (bind, listen, accept, connect)
- *Lesson: Copying code teaches patterns, but understanding requires implementation*

**Week 2: UDP & Multi-Client Servers**
- Built UDP sender/listener with packet loss awareness
- Created poll-based multi-client TCP server
- Experienced the fundamental difference between TCP's reliability and UDP's speed
- *Lesson: Continuing through tedious work is harder than starting*

**Week 3: Reliable UDP (Incomplete)**
- Attempted to implement reliability layer on top of UDP
- Explored ACK/NACK systems and bitmap-based packet tracking
- Got tangled in retransmission logic without proper planning
- Created buffer manipulation utilities for binary packing/unpacking
- *Lesson: Complex systems need architecture-first approaches*

**Week 4: Multi-Client Chat Server**
- Planned architecture before writing code
- Built complete TCP chat server with:
  - User registration and custom usernames
  - Broadcast messaging to all clients
  - Direct messaging (@username)
  - Command system (/users, /myname, /help)
- Modular design with separated utility files for scalability
- *Lesson: Time spent planning saves multiples in implementation*

### Month 2: Game Networking (January-February 2025)
**Status: In Progress. Building real-time multiplayer systems.**

**Week 5: Multiplayer Movement Server**
- Built UDP-based game server with 30Hz tick-based updates
- Implemented binary network protocol with efficient packet packing
- Multiple clients moving simultaneously with real-time position synchronization
- Non-blocking terminal input for smooth client-side controls
- Binary protocol: 2-byte App ID + 2-byte Message ID + payload
- *Lesson: Reading about game networking feels different than implementing it*

**Week 6: Multiplayer Treasure Hunt Game**
- Transformed movement server into competitive treasure-hunting game
- **Game features:**
  - 100x100 map with dynamic 10x10 viewport following player
  - Up to 80 concurrent treasures with varying point values (1-3 pts)
  - Server-authoritative collision detection
  - Real-time scoreboard
  - Connection limits (max 12 players) with graceful rejection
  - AI bot mode with Euclidean distance-based pathfinding
- **Technical accomplishments:**
  - ~1,628 lines of modular C code across 6 utility files
  - Linked-list structures for players and treasures
  - Checkerboard viewport rendering for visual movement feedback
- *Lessons: Always guard NULL pointers. stdout buffering hides debug output during segfaults—use stderr or fflush(stdout).*

**Week 7: UDP Proxy & Network Simulator**
- Built transparent UDP proxy to simulate real-world network conditions
- **Challenge:** Preserving destination addresses in connectionless UDP routing
- **Solution:** Wrapper functions (send_proxy/rec_proxy) that embed routing info in packet data
- **Proxy features:**
  - Configurable latency simulation (command-line adjustable)
  - Packet loss simulation (percentage-based)
  - Real-time statistics (packets received/forwarded/dropped)
  - Delayed packet queue with millisecond precision
- **Integration:** Find-and-replace sendto/recvfrom—works with any UDP application
- **Testing insights:**
  - 50ms latency made treasure hunt feel sluggish
  - 100ms+ latency became unplayable without client prediction
  - Packet loss broke game state synchronization
  - Critical events (connections, treasure collection) need reliability
- Implemented two versions: v1 (proof of concept) and v2 (production-ready with stats)
- *Lesson: Network conditions expose architectural weaknesses. Client prediction and state reconciliation aren't optional—they're fundamental.*

---

## What I'm Building Toward

**The skill:** Real-time multiplayer systems engineering.

Most developers can build CRUD applications. Fewer can build responsive real-time networked systems that feel instant despite network latency. That's what I'm learning to do.

**By the end of 6 months:**
- Deep understanding of game networking (state sync, client prediction, lag compensation, interest management)
- Portfolio of working multiplayer systems built from first principles
- Strong C programming foundation and comfort with low-level systems
- Proven ability to ship projects under self-imposed constraints
- 120 days of consecutive deep work proving consistency
- The confidence to tackle complex distributed systems problems

**Why this matters:**
Every multiplayer game, real-time collaboration tool, and distributed system faces the same fundamental problems: unreliable networks, latency, and state consistency. Learning to solve these problems at the protocol level builds intuition that applies everywhere.

---

## Tech Stack

**Language:** C (learning low-level fundamentals)

**Networking:** 
- Berkeley sockets (POSIX)
- UDP for real-time state updates
- TCP for reliable messaging
- Poll-based I/O multiplexing

**Tools:**
- GCC compiler
- Make for builds
- Git for version control
- Terminal for everything

**Platform:** Linux (Ubuntu 24)

---

## Key Concepts Explored

**Month 1: Protocol Fundamentals**
- TCP vs UDP tradeoffs (reliability vs speed)
- Socket programming (bind, listen, accept, connect, sendto, recvfrom)
- Poll-based I/O multiplexing for handling multiple clients
- Client/server architecture patterns
- Protocol design (message parsing, command systems)
- Binary data manipulation (bit packing/unpacking)

**Month 2: Real-Time Game Networking**
- Tick-based game loops (30Hz server updates)
- State synchronization across multiple clients
- Binary network protocols (efficient 2-byte headers + payload)
- Non-blocking terminal input handling
- Game state management (linked-list structures for dynamic entities)
- Dynamic viewport rendering with coordinate transformations
- Server-authoritative collision detection and validation
- Pathfinding algorithms (Euclidean distance-based nearest-neighbor)
- Network condition simulation (latency, packet loss)
- Transparent proxy design for UDP routing
- Wrapper functions for protocol abstraction

---

## Running the Projects

Each week's project has its own README with build and run instructions.

**General pattern:**

```bash
cd week#_project_name/
gcc ... # specific gcc cmd for each file, found in that project's readme
./server      # Terminal 1
./client      # Terminal 2
./client      # Terminal 3 (for multi-client projects)
```

---

## Learning Resources

**Primary sources:**
- Beej's Guide to Network Programming
- Glenn Fiedler's Gaffer on Games articles (Networking section)
- Gabriel Gambetta's Client-Server Game Architecture series

**Approach:**
Read for understanding, then build from scratch. No copy-paste. If I don't understand it well enough to implement it myself, I don't understand it.

---

## The Rules

1. **Show up every day.** 2pm. 2.5 hours. No exceptions.
2. **No phone during sessions.** Entirely focused on one thing.
3. **Build, don't just read.** Concepts mean nothing until implemented.
4. **Document the journey.** Architecture notes, code comments, reflections on what worked.
5. **Ship projects.** Incomplete is acceptable if lessons are documented.
6. **Plan before coding.** Week 3's tangled bitmap logic taught me this the hard way.

---

## Progress Tracking

**Days completed:** 33/120 (Month 2, Week 7)

**Current streak:** 33 days

**Projects shipped:** 5
- TCP Chat Server (Week 4)
- Multiplayer Movement Server (Week 5)
- Treasure Hunt Game (Week 6)
- UDP Proxy v1 (Week 7)
- UDP Proxy v2 with Statistics (Week 7)

**Lines of code written:** ~3,500+ (not counting iterations and rewrites)

---

## Why Public?

Accountability.

Making this public means I can't quietly quit when it gets hard. It also means I write code that I'm not embarrassed to show, which raises the bar.

If you're reading this and considering your own deep work project: just start. The first day is the hardest. The second day is hard too. By day 20, showing up at 2pm is just what you do. By day 30, not showing up feels wrong.

The consistency compounds.

---

**Last updated:** February 11, 2026 (Month 2, Week 7)

**Next challenge:** Making the game feel responsive even with bad network conditions.

---

## What's Next

**Week 8 (Planned):** Client prediction and interpolation
- Implement client-side prediction for player movement
- Server reconciliation when predictions diverge
- Entity interpolation for smooth remote player movement
- Make the treasure hunt playable with 100ms+ latency

**Week 9 (Planned):** Lag compensation
- Server-side rewind for hit detection
- Client-server time synchronization
- Input buffering and replay

**Month 3 (Planned):** Advanced multiplayer concepts
- Interest management (only send relevant updates)
- Delta compression (send only what changed)
- Snapshot interpolation
- More sophisticated protocols

**Months 4-6:** TBD based on where the journey leads