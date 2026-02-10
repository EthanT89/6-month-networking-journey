# UDP Proxy - Week 7

Day 31. Built a UDP proxy that can sit between any client and server without modifying their packet protocols.

## The Problem

When I started thinking about how to build a latency simulator, I ran into a fundamental issue with UDP.

```
Client → Proxy → Server
```

Client sends packet to Proxy. Proxy needs to forward it to Server. But here's the problem: the destination address in the UDP packet is the Proxy's address, not the Server's. The Proxy has no way to know where the Client actually wanted to send the packet.

I thought about a few solutions:
1. Hardcode routes in the proxy (port 1210 always goes to port 1209) - Works, but inflexible. Only works for one specific setup.
2. Add destination address to every packet in the game protocol - Effective, but requires rewriting a ton of code in every application I want to test.
3. Track connections and map them - Complex state management that breaks with multiple servers.

None of these felt right. The goal was to make this as easy as possible to integrate into any UDP project.

## The Solution

I realized I could create wrapper functions that look exactly like standard `sendto()` and `recvfrom()`, but handle the destination address internally.

**Client code changes:**
```c
// Before:
sendto(sockfd, buffer, len, 0, &dest_addr, addr_len);

// After:
send_proxy(sockfd, buffer, len, 0, &dest_addr, addr_len);
```

Same arguments. Same behavior. Just a different function name.

To integrate this into an entire codebase:
```bash
sed -i 's/sendto(/send_proxy(/g' *.c
sed -i 's/recvfrom(/rec_proxy(/g' *.c
```

Takes 5 seconds. Takes 5 seconds to remove it too.

## How It Works

**send_proxy():**
- Takes the same arguments as standard `sendto()`
- Prepends the destination address to the packet data: `[dest_addr | original_data]`
- Sends the whole thing to the proxy (hardcoded to 127.0.0.1:5050)

**Proxy (proxy.c):**
- Receives packet: `[dest_addr | original_data]`
- Extracts the real destination address from the packet
- Gets the sender's address from the OS (automatic with UDP)
- Repacks as: `[sender_addr | original_data]`
- Forwards to the real destination

**rec_proxy():**
- Receives packet from proxy: `[sender_addr | original_data]`
- Extracts the sender address
- Returns the original data and sender address to the application
- Application sees normal UDP behavior, doesn't know proxy exists

## Why This Works

The key insight: separate where the packet physically goes from where the sender wants it to go.

UDP routing only knows the immediate next hop. By embedding the destination in the data itself, I preserved the sender's intent while routing through the proxy.

This means:
- Client thinks it's talking directly to Server
- Server thinks it's talking directly to Client
- Proxy is invisible (except for the function name change)

Works for:
- Multiple clients → one server
- One client → multiple servers
- Multiple clients → multiple servers
- Any UDP application

## Usage

**1. Start the proxy:**
```bash
gcc proxy.c utils/proxy_utils.c -o proxy
./proxy
```

Proxy listens on port 5050.

**2. Modify your application:**

Include the proxy utils:
```c
#include "proxy_utils.h"
```

Change function names:
- `sendto()` → `send_proxy()`
- `recvfrom()` → `rec_proxy()`

**3. Run your application**

Everything routes through the proxy automatically.

## Testing

I had Copilot generate test_server.c and test_client.c since my focus was on the proxy concept, not writing test applications.

**Terminal 1:**
```bash
./proxy
```

**Terminal 2:**
```bash
./test_server
```

**Terminal 3:**
```bash
./test_client
```

Client sends messages. Server receives them as if they came directly, even though they're routing through the proxy.

## Current Status (Day 31)

**What works:**
- Transparent packet forwarding
- Address preservation (sender and destination)
- Bidirectional communication
- Multiple clients, multiple servers

**What's next:**
- Day 32: Add delay queue (configurable latency)
- Day 33: Packet loss simulation
- Day 34: Test with Week 6 game
- Day 35: Documentation and reflection

## Files

- `proxy_config.h` - Configuration (proxy port)
- `proxy_utils.h/c` - Wrapper functions
- `proxy.c` - Main proxy server
- `test_server.c` - Test server
- `test_client.c` - Test client

## Configuration

Edit `proxy_config.h` to change the proxy port:
```c
#define PROXY_PORT_S "5050"  // String for getaddrinfo()
#define PROXY_PORT_N 5050    // Integer for htons()
```

## Limitations

**Right now:**
- Localhost only (127.0.0.1)
- No delay or packet loss yet (coming tomorrow)
- Fixed proxy port

**By design:**
- Adds ~16 bytes overhead per packet (for address data)
- Requires changing function names (not completely transparent)
- Assumes MAXBUFSIZE is big enough for address + data

## Why I Built It This Way

I prioritized ease of use and flexibility. The minimal code changes mean this can work with any UDP project. The transparent routing means I can test network conditions without rewriting application logic.

Tomorrow I'll add the delay queue. The routing and address preservation was the hard part. Adding delay is just queuing packets with timestamps and sending them when time's up.

This is a pretty basic implementation. I was focused on getting the concept working. There's room for improvement, but it's a solid foundation.

---

**Day 31 complete. Proxy routing works. Tomorrow: add the delay.**