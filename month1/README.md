# Month 1: Networking Fundamentals

The foundation month. Four weeks of learning socket programming, understanding TCP vs UDP, and building progressively more complex servers. This was all about getting comfortable with the basics before attempting real-time multiplayer systems.

---

## Weekly Breakdown

### Week 1-2: TCP and UDP Basics
**Folder:** [Week1-2/](Week1-2/)

The beginning. I followed Beej's Guide closely, typing out examples and experimenting. Started with basic TCP echo servers, moved to poll-based multi-client servers, then explored UDP.

**Key projects:**
- TCP echo server with fork()
- Poll-based multi-client chat server
- UDP sender/listener pair

**Lesson:** Copying code teaches patterns, but understanding requires experimentation.

---

### Week 3: Reliable UDP
**Folder:** [Week3/](Week3/)

Attempted to implement TCP-like reliability on top of UDP using ACK bitmaps and selective retransmission. Got tangled in the complexity without proper planning.

**Key concepts:**
- Sliding window ACK protocol
- Bit manipulation for bitmap tracking
- Retransmission with timeouts

**Lesson:** Complex systems need architecture-first approaches. Diving straight into implementation leads to technical debt and subtle bugs.

---

### Week 4: Multi-Client Chat Server
**Folder:** [Week4/](Week4/)

The first project I planned before coding. Built a complete TCP chat server with usernames, broadcast messaging, direct messaging (@username), and a command system (/users, /help, etc.).

**Key features:**
- Modular design with separate utility files
- Command parsing and handling
- Direct messaging between users
- Graceful connection handling

**Lesson:** Time spent planning saves multiples in implementation. This was the smoothest project of the month.

---

## Shared Utilities

### [utils/](utils/)

Common modules extracted from the weekly projects:
- **buffer_manipulation.c/h** - Binary packing/unpacking (packi16, unpacki32, etc.)
- **ack.c/h** - ACK bitmap logic for reliable UDP
- **client.c/h** - Linked list for tracking connected clients
- **pfds.c/h** - Helper functions for managing poll() file descriptors

These utilities evolved organically as I kept rewriting the same code. Eventually I pulled common patterns into reusable modules.

---

## Overall Progress

**Days spent:** 20 consecutive days (Jan 13 - Feb 7, 2025)

**Lines of code:** ~2,000 (including rewrites and experiments)

**Projects completed:**
1. TCP echo server
2. Poll-based multi-client server
3. UDP sender/listener
4. Reliable UDP protocol (incomplete)
5. TCP chat server (fully functional)

**Major takeaways:**
- TCP and UDP serve fundamentally different purposes
- poll() is essential for multi-client servers
- Binary protocols require careful byte-level manipulation
- Planning complex systems upfront prevents technical debt
- Failing at Week 3 taught as much as succeeding at Week 4

---

## What's Next

Month 2 applies these fundamentals to real-time game networking. The socket programming, poll-based I/O, and binary packing all become tools for building multiplayer systems.

See [../week1/README.md](../week1/README.md) for Month 2, Week 1 (Multiplayer Movement Server).

---

## Additional Documentation

- [month1_learnings.md](month1_learnings.md) - Detailed reflections and lessons from each week
- [structs.c](structs.c) - Experiments with C structs and memory management

---

**Status: Month 1 complete. Foundation solid. Ready for real-time systems.**
