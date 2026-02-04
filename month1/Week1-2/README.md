# Week 1-2: TCP and UDP Fundamentals

The beginning of the journey. These first two weeks were all about getting comfortable with sockets—understanding what they are, how they work, and the difference between TCP and UDP.

I followed Beej's Guide to Network Programming pretty closely here. A lot of this code is adapted from his examples, but I made sure to type everything out myself, add comments, and experiment. Copying code teaches patterns. Understanding comes from breaking things.

---

## What's In Here

This folder has a few different mini-projects, each building on the last:

### TCP Echo Server (`server.c` + `client.c`)

The classic first socket program. The server listens for a connection, sends "Hello, World!", receives a message back, and prints it. The client connects, receives the greeting, sends a message, and exits.

**Key concepts:**
- Basic socket setup (`socket()`, `bind()`, `listen()`, `accept()`)
- The `fork()` pattern for handling multiple clients
- Signal handling to reap zombie processes
- Why `setsockopt(SO_REUSEADDR)` matters when restarting servers

**Build and run:**
```bash
gcc server.c -o server
gcc client.c -o client

# Terminal 1
./server

# Terminal 2
./client localhost
```

---

### Multi-Client Poll Server (`pollserver.c` + `pollclient.c`)

This is where things get interesting. Instead of forking a new process for each client, we use `poll()` to handle multiple clients in a single thread. It's a simple chat server—anything one client sends gets broadcast to all the others.

**Key concepts:**
- `poll()` for I/O multiplexing (way cleaner than `select()`)
- Dynamic array management for file descriptors
- Non-blocking server architecture
- The difference between blocking on `accept()` vs blocking on `poll()`

**Build and run:**
```bash
gcc pollserver.c -o pollserver

# Terminal 1
./pollserver

# Terminal 2 (and 3, 4, etc.)
telnet localhost 9034
```

Type messages in any telnet window and watch them appear in the others. This was the first moment it felt like I was building something real.

---

### UDP Listener/Sender (`udp_listener.c` + `udp_sender.c`)

After all that TCP, I wanted to see how UDP felt different. Spoiler: it's simpler to set up, but you lose all the guarantees.

**Key concepts:**
- `SOCK_DGRAM` instead of `SOCK_STREAM`
- `recvfrom()` and `sendto()` (no connection needed!)
- Basic sequence numbers for ordering
- Why UDP is used for games (hint: it's about latency, not reliability)

**Build and run:**
```bash
gcc udp_listener.c utils/buffer_manipulation.c -o udp_listener
gcc udp_sender.c utils/buffer_manipulation.c -o udp_sender

# Terminal 1
./udp_listener

# Terminal 2
./udp_sender localhost
```

Note: The UDP examples use some buffer packing utilities from the parent `utils/` folder. This was the first time I started extracting reusable code into separate files.

---

### Other Files

- `server_multiclient.c` — An earlier attempt at multi-client handling (before I discovered `poll()`)
- `buffer` — A compiled binary, probably should be in `.gitignore`
- `test_text` — Test data file

---

## What I Learned

**Week 1** was about getting the basic patterns into my head. Every socket program follows the same dance: create socket, set options, bind (for servers), listen/accept or connect, send/receive, close. Once you see it a few times, it clicks.

**Week 2** was about understanding *why* we need abstractions like `poll()`. Forking works, but it's heavy. Non-blocking I/O with multiplexing is how real servers handle thousands of connections.

The biggest takeaway: TCP handles so much for you (ordering, reliability, flow control). UDP gives you nothing but speed. Both have their place.

---

## Next Steps

These fundamentals led directly into Week 3 (Reliable UDP) and Week 4 (Chat Server). The poll server code became the foundation for everything that followed.
