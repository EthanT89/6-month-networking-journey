# UDP Proxy v1 - Transparent Routing Proof of Concept

Day 31. Built a UDP proxy that can sit between any client and server without modifying their packet protocols. This is the initial proof-of-concept—routing works, but no latency simulation yet.

## The Problem

When I started thinking about building a latency simulator, I ran into a fundamental issue with UDP:

```
Client → Proxy → Server
```

Client sends packet to Proxy. Proxy needs to forward it to Server. But here's the problem: the destination address in the UDP packet becomes the Proxy's address, not the Server's. The Proxy has no way to know where the Client actually wanted to send the packet.

I considered several solutions:
1. **Hardcode routes** (port 1210 always forwards to port 1209) - Works for one specific setup, but completely inflexible
2. **Modify packet protocols** (embed destination in every packet) - Effective, but requires rewriting application code
3. **Track connections and map them** - Complex state management that breaks with multiple servers

None of these felt right. The goal was seamless integration with any UDP application.

## The Solution

Create wrapper functions that look identical to standard `sendto()` and `recvfrom()`, but handle destination routing internally.

**Code changes required:**
```c
// Before:
sendto(sockfd, buffer, len, 0, &dest_addr, addr_len);

// After:
send_proxy(sockfd, buffer, len, 0, &dest_addr, addr_len);
```

Same arguments. Same behavior. Different function name.

**Integrating into existing code:**
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

**What's missing:**
- Latency simulation (coming in v2)
- Packet loss simulation (coming in v2)
- Statistics tracking (coming in v2)

This version proves the routing concept works. v2 adds the network simulation features.

---

## Files

- `proxy_config.h` - Configuration (proxy port)
- `proxy_utils.h/c` - Wrapper functions (send_proxy/rec_proxy)
- `proxy.c` - Main proxy server
- `test_server.c` - Test server application
- `test_client.c` - Test client application

---

## Configuration

Edit `proxy_config.h` to change the proxy port:
```c
#define PROXY_PORT_S "5050"  // String for getaddrinfo()
#define PROXY_PORT_N 5050    // Integer for htons()
```

---

## Limitations

**Current version:**
- Localhost only (127.0.0.1)
- No delay or packet loss (see v2 for that)
- Fixed proxy port

**By design:**
- Adds ~16 bytes overhead per packet (for address data)
- Requires changing function names (not completely transparent)
- All packets routed through single proxy port

---

## Why I Built It This Way

I prioritized proving the routing concept first. The minimal code changes mean this can work with any UDP project. The transparent routing means I can test network conditions without rewriting application logic.

The routing and address preservation was the hard part. Once that worked, adding latency and packet loss (in v2) became straightforward—just queue packets with timestamps and send when time's up.

---

**Status: v1 complete. Transparent routing works. See v2 for network simulation features.**