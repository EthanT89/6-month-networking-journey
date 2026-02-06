# Real-Time Systems From Scratch

**A 6-month deep work project to learn real-time networked systems in C.**

20 years old. Computer Science and Engineering student at UC Irvine. Software Engineer at First American

I'm learning how multiplayer games actually work by building networking systems from the ground up—no frameworks, no game engines, just C and the fundamentals.

---

## What This Is

This repository contains everything I've built during my 6-month commitment to learning real-time networked systems. Every weekday at 2pm, I sit down for 2.5 hours of focused work. No phone. No distractions. Just building.

I'm documenting the entire journey through weekly blog posts and this code. The goal isn't just to learn networking concepts—it's to build the discipline to show up every single day and do hard things.

---

## The Journey So Far

### Month 1: Fundamentals (January 2025)
**Status: Complete. 20 days straight. Zero missed.**

**Week 1: TCP Basics**
- Worked through Beej's Guide to Network Programming
- Built basic TCP echo server
- Learned socket programming fundamentals
- *Lesson: Copying code teaches patterns*

**Week 2: UDP & Packet Loss**
- Built UDP server with packet loss simulation
- Experienced the difference between TCP and UDP
- Pushed through exhaustion (3:01pm moment)
- *Lesson: Continuing is harder than starting*

**Week 3: Reliable UDP (Incomplete)**
- Attempted to build reliability on top of UDP
- Got tangled in bitmaps, ACKs, and retransmission logic
- Learned what happens when you don't plan
- *Lesson: Planning prevents tedium*

**Week 4: Multi-Client Chat Server**
- Planned architecture before coding
- Built clean, working TCP chat server with usernames, broadcast, direct messaging, and commands
- Shipped a complete project
- *Lesson: Planning pays off*

### Month 2: Game Networking (January-February 2025)
**Status: In Progress**

**Week 5: Multiplayer Movement Server**
- Built UDP game server with tick-based updates
- Implemented client-side interpolation for smooth movement
- Multiple clients moving in real-time, seeing each other's positions
- Binary network protocol with efficient packing
- *Lesson: Applying fundamentals feels different than learning them*

**Week 6: Multiplayer Treasure Hunt Game**
- Transformed movement server into a competitive treasure hunting game
- Dynamic treasure spawning system (up to 80 concurrent treasures with varying point values)
- Moving 10x10 viewport that follows player on 100x100 map
- Server-side collision detection and validation
- AI bot mode with pathfinding to nearest treasure
- Connection limits (max 12 players) with graceful rejection handling
- ~1,628 lines of modular C code across 6 utility files
- *Lesson: NULL pointer guards everywhere, and stdout buffering will betray you during segfaults*

---

## What I'm Building Toward

**The skill:** Real-time multiplayer systems engineering.

Most developers can build CRUD apps. Few can build real-time networked systems that feel responsive even with latency. That's what I want to be good at.

By the end of 6 months:
- Deep understanding of game networking (state sync, client prediction, lag compensation)
- Portfolio of working multiplayer systems
- Strong C programming foundation
- Ability to ship projects under time constraints
- 120+ days of consecutive deep work

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

**Month 1:**
- TCP vs UDP tradeoffs
- Socket programming (bind, listen, accept, connect)
- Poll-based I/O for handling multiple clients
- Client/server architecture
- Protocol design

**Month 2:**
- Tick-based game loops
- State synchronization
- Binary network protocols (bit packing/unpacking)
- Client-side interpolation
- Non-blocking input handling
- Game state management (linked-list structures for players and treasures)
- Dynamic viewport rendering
- Server-authoritative collision detection
- Pathfinding algorithms (Euclidean distance-based)

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
4. **Document the journey.** Weekly blog posts, architecture notes, code comments.
5. **Ship projects.** Incomplete is fine if documented. Undocumented is not.
6. **Plan before coding.** Week 3 taught me this the hard way.

---

## Why Public?

Accountability. 

Making this public means I can't quietly quit when it gets hard. It also means I have to write code I'm not embarrassed to show.

If you're reading this and thinking about starting your own deep work project: just start. The first day is the hardest. The second day is hard too. By day 20, it's just what you do at 2pm.

---

## Progress Tracking

**Days completed:** 30/180 (Month 2, Day 30)

**Current streak:** 30 days

**Projects shipped:** 3 (Chat server, Movement server, Treasure Hunt game)

**Blog posts:** 5 (Weeks 1-5)

---

## What's Next

**Week 7 (Planned):** Client prediction - Make your own movement feel instant while still syncing with server

**Week 8 (Planned):** Networking tools - Build profilers, visualizers, or lag simulators

**Month 3-6:** TBD - Depends on what I learn in Month 2

---

## Connect

I'm documenting this journey through weekly blog posts. If you want to follow along or have questions about the projects, feel free to reach out.

This is hard. But it's worth it.

---

**Last updated:** February 6, 2026 (Month 2, Week 6)

**Next session:** Monday, 2pm, same table.