# Week 3: Reliable UDP Implementation

An attempt to build reliability on top of UDP using ACK bitmaps and selective retransmission. This was my first complex networking project, and it taught me why planning matters.

---

## What This Does

Implements reliable packet delivery over UDP using:
- **Sliding window ACK bitmap** (32-packet window)
- **Selective retransmission** (1-second timeout)
- **Packet loss simulation** (configurable drop rate for testing)

The goal: understand how TCP's reliability features work under the hood by implementing a simplified version myself.

---

## How It Works

### The Protocol

**Client sends:**
```
[Protocol ID: 4 bytes][Packet ID: 4 bytes][Message: variable]
```

**Server responds with ACK:**
```
[Protocol ID: 4 bytes][Most Recent Packet ID: 4 bytes][32-bit ACK Bitmap: 4 bytes]
```

### ACK Bitmap Explanation

The bitmap tracks the last 32 packets:
- Bit 0 = most recent packet received
- Bit 1 = packet (most_recent - 1)
- Bit 31 = packet (most_recent - 31)

Example: If the server received packets 100, 99, 97, 96 (but missed 98), the bitmap would be:
```
Most Recent ID: 100
Bitmap: 11011 (in the last 5 bits)
```

When the client gets this ACK, it knows:
- Packet 100: ACK'd
- Packet 99: ACK'd
- Packet 98: Missing (needs retransmit)
- Packet 97: ACK'd
- Packet 96: ACK'd

### Retransmission Logic

The client tracks every sent packet with a timestamp. Every loop:
1. Check for new ACK messages from server
2. Update the ACK bitmap based on received ACKs
3. For each unACK'd packet older than 1 second, retransmit

---

## What I Learned

This project was humbling. I jumped straight into implementation without planning the state machine or edge cases. The result? A tangled mess that *mostly* worked but had subtle bugs.

**Key lessons:**

**Bit manipulation is tricky.** Shifting, masking, handling byte order—C doesn't make this easy. I spent hours debugging off-by-one errors in the bitmap logic.

**Planning prevents tedium.** I kept adding features reactively: "Oh, I need to track timestamps." "Oh, I need to handle wraparound." Halfway through, I wished I'd spent a day diagramming the protocol.

**Edge cases matter.** What happens when packet IDs wrap? What if an ACK arrives after we've moved the window past it? What if the client disconnects mid-stream? I didn't think about these upfront.

**Testing is hard without tools.** I manually set packet drop rates and watched console output. Should've built a proper test harness.

---

## Known Issues

This implementation has a critical flaw: **the bitmap assumes packets arrive in order.**

If the server receives packet 102, then 100 (retransmitted), the bitmap logic gets confused because 100 < 102. The bitmap shifts forward, and packet 100 gets marked as "outside the window" instead of ACK'd.

The fix is straightforward—check if an incoming packet is a retransmit and handle it separately—but I moved on to Week 4 rather than polish this. The goal was learning, and I learned plenty.

---

## Build & Run

### Compile
```bash
gcc udp_client_vc.c ../utils/buffer_manipulation.c ../utils/ack.c -o udp_client
gcc udp_server_vc.c ../utils/buffer_manipulation.c ../utils/ack.c -o udp_server
```

### Run

**Terminal 1 (Server):**
```bash
./udp_server
```

**Terminal 2 (Client):**
```bash
./udp_client localhost
```

**Testing packet loss:**

Edit `udp_server_vc.c` and change:
```c
#define PACKET_DROP_RATE 30  // 30% packet loss
```

Recompile and watch the retransmission logic work. You'll see packets being resent after 1 second timeouts.

---

## Files

- `udp_client_vc.c` - Client with ACK handling and retransmission
- `udp_server_vc.c` - Server with ACK bitmap and packet loss simulation
- `week3_reflection.md` - Detailed notes on what went wrong and why
- `udp_testing` - Compiled binary (should probably be in .gitignore)

---

## Reflections

This was my first real taste of protocol design. TCP makes this look easy because decades of smart people solved all these problems. Doing it yourself—even a simplified version—reveals just how many edge cases exist.

Would I do it differently? Absolutely. Next time: diagram the state machine, list all edge cases, write pseudocode, *then* write C. Week 4's chat server proved this approach works.

But failing here was valuable. You learn more from a messy, half-working implementation than from perfectly following a tutorial.

---

**Week 3 Status: Incomplete but educational.**

The code works well enough to demonstrate the concepts. The bugs teach as much as the working parts.
