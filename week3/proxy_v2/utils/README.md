# Proxy v2 Utilities

Helper modules for the network simulation proxy with latency and packet loss features.

---

## What's Here

### Proxy Utils (`proxy_utils.c/h`)

Identical to proxy_v1's utilities—these are the transparent routing wrapper functions. See [../../proxy_v1/utils/README.md](../../proxy_v1/utils/README.md) for full documentation.

**Quick summary:**
- `send_proxy()` - Drop-in replacement for `sendto()` that routes through proxy
- `rec_proxy()` - Drop-in replacement for `recvfrom()` that handles proxy routing
- `prepend_address()` - Internal helper for packing address data

---

### Delayed Packet Queue (`delayed_packet.c/h`)

The star of v2—implements latency simulation and packet loss through a queue-based delay system.

**Structures:**

**`struct Packet`** - Represents a single packet waiting to be sent
```c
struct Packet {
    unsigned char data[MAXBUFSIZE];  // Packet contents
    int bytes;                       // Length of data
    struct sockaddr *dest_addr;      // Where to send it
    socklen_t dest_addrlen;          // Address length
    int time_received;               // When proxy received it (ms)
    struct Packet *next;             // Linked list pointer
};
```

**`struct Packets`** - The packet queue
```c
struct Packets {
    int delay_ms;                    // Latency to simulate (ms)
    int drop_rate;                   // Packet loss percentage (0-100)
    int pkt_count;                   // Current queue size
    struct Packet *head;             // First packet in queue
    struct Packet *tail;             // Last packet in queue
};
```

---

### Key Functions

**Queue Management:**

**`enqueue_packet()`**
```c
void enqueue_packet(struct Packets *packets, struct Packet *packet);
```
- Adds packet to the end of the delay queue
- Tracks timestamp for delay calculation
- Thread-safe for single-threaded proxy

**`pop_packet()`**
```c
struct Packet* pop_packet(struct Packets *packets);
```
- Removes and returns the first packet in queue
- Caller responsible for freeing packet memory
- Returns NULL if queue is empty

**`ready_to_send()`**
```c
int ready_to_send(struct Packets *packets);
```
- Checks if head packet has waited long enough
- Compares `current_time - time_received >= delay_ms`
- Handles millisecond wraparound correctly
- Returns 1 if ready, 0 if not

---

## How Latency Simulation Works

**The delay queue creates artificial latency:**

1. **Packet arrives at proxy** (time: 0ms)
   - Proxy receives packet via `recvfrom()`
   - Timestamp recorded: `time_received = get_time_ms()`
   - Packet added to queue via `enqueue_packet()`

2. **Packet waits in queue** (time: 0ms → delay_ms)
   - Every loop, proxy calls `ready_to_send()`
   - If `(current_time - time_received) < delay_ms`: keep waiting
   - If `>= delay_ms`: packet is ready

3. **Packet sent** (time: delay_ms)
   - Proxy pops packet from queue
   - Random number generator determines if packet drops
   - If not dropped: `sendto()` forwards to destination
   - Packet memory freed

---

## Packet Loss Simulation

Before sending each packet, the proxy rolls the dice:

```c
if ((rand() % 100 + 1) < packets->drop_rate) {
    // Packet lost! Drop it without sending
    stats->packets_dropped++;
    return;
}

// Packet survives! Send it
sendto(sockfd, packet->data, packet->bytes, 0, packet->dest_addr, packet->dest_addrlen);
stats->packets_forwarded++;
```

**Example:** With `drop_rate = 10`:
- `rand() % 100 + 1` generates 1-100
- If result is 1-10 (10% chance): packet drops
- If result is 11-100 (90% chance): packet sends

---

## Timing Details

**Millisecond precision with wraparound:**

The proxy uses `get_time_ms()` from `time_custom.c`, which returns 0-999 (wraps every second).

**Handling wraparound:**
```c
// If current_time < time_received, we wrapped
if (current_time < time_received) {
    elapsed = (1000 - time_received) + current_time;
} else {
    elapsed = current_time - time_received;
}

return elapsed >= delay_ms;
```

**Why wrap at 1000ms?** Simplifies the code. For delays up to 999ms, this works perfectly. For longer delays, the system would need 64-bit timestamps.

---

## Memory Management

**Important:** The delayed packet queue dynamically allocates memory for:
- Each `struct Packet` (via `malloc()`)
- Each `packet->dest_addr` (via `malloc()`)

**Cleanup responsibility:**
- `pop_packet()` removes from queue but doesn't free memory
- Caller must `free(packet->dest_addr)` and `free(packet)` after sending
- The proxy's `send_packet()` handles this correctly

**Potential leak:** If proxy crashes or is killed (SIGKILL), queued packets leak. For a local testing tool, this is acceptable. Production systems would need graceful shutdown.

---

## Configuration

Controlled via `proxy_config.h`:

```c
#define DELAY_MS 50        // Default latency
#define DROP_RATE 10       // Default packet loss (%)
```

Or via command-line arguments (see main proxy README).

---

## Queue Behavior

**FIFO (First-In-First-Out):** Packets are sent in the order received (after delay).

**No prioritization:** All packets treated equally. Production systems might prioritize critical messages (e.g., player connections over position updates).

**Unbounded:** No maximum queue size. With very high latency and packet rates, memory could grow unbounded. For local testing with realistic parameters, this isn't an issue.

---

## Performance Characteristics

**Time complexity:**
- `enqueue_packet()`: O(1) - append to tail
- `ready_to_send()`: O(1) - check head timestamp
- `pop_packet()`: O(1) - remove head

**Space complexity:** O(n) where n = number of packets in flight

With 100ms latency and 30 packets/second, queue typically holds ~3 packets. Even at 500ms delay with 100 packets/second, queue holds ~50 packets (a few KB of memory).

---

## Why This Design?

**Could've used:** Priority queue, circular buffer, or timestamp-indexed array.

**Why linked list?** Simple, dynamic, and efficient enough for the scale. The bottleneck is network I/O, not queue operations. KISS principle applies.

---

**These utilities enable v2's network simulation features. Combined with proxy_utils.c for routing, they create a complete latency/packet loss testing tool.**
