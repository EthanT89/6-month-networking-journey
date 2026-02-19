# Proxy v1 Utilities

Helper functions for the transparent UDP proxy routing system.

---

## What's Here

### Proxy Utils (`proxy_utils.c/h`)

The core wrapper functions that enable transparent UDP routing through the proxy. These functions mirror the standard `sendto()` and `recvfrom()` signatures but embed routing information in the packet data.

**Functions:**

**`send_proxy()`**
```c
int send_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, 
               int flags, struct sockaddr *addr, socklen_t addr_len);
```
- Mirrors `sendto()` signature exactly
- Prepends destination address to packet data: `[dest_addr | original_data]`
- Sends enriched packet to proxy (hardcoded to 127.0.0.1:5050)
- Returns bytes sent (or -1 on error)

**`rec_proxy()`**
```c
int rec_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, 
              int flags, struct sockaddr *addr, socklen_t *addr_len);
```
- Mirrors `recvfrom()` signature exactly
- Receives packet from proxy: `[sender_addr | original_data]`
- Extracts sender address and original data
- Returns original data length (or -1 on error)

**`prepend_address()`**
```c
int prepend_address(struct sockaddr *addr, unsigned char packet[MAXBUFSIZE], 
                    unsigned char buf[MAXBUFSIZE], size_t buflen);
```
- Internal helper that packs address data before message content
- Copies `sa_data` (14 bytes) and `sa_family` (2 bytes) to packet
- Returns total packet size after prepending

---

## How It Works

### Packet Flow

**Sending (client → proxy → server):**

1. Client calls: `send_proxy(sockfd, "hello", 5, 0, &server_addr, addr_len)`
2. `send_proxy()` prepends server address: `[server_addr(16 bytes) | "hello"(5 bytes)]`
3. Enriched packet (21 bytes) sent to proxy at 127.0.0.1:5050
4. Proxy extracts `server_addr` from packet
5. Proxy replaces with `client_addr` (from OS)
6. Proxy forwards: `[client_addr(16 bytes) | "hello"(5 bytes)]` to server
7. Server calls: `rec_proxy(sockfd, buf, 1024, 0, &sender_addr, &sender_len)`
8. `rec_proxy()` extracts `client_addr` and "hello"
9. Server sees: sender = client_addr, message = "hello"

**The magic:** Applications think they're talking directly to each other. The proxy is invisible except for the function name change.

---

## Address Format

`struct sockaddr` is packed as:
```
[sa_data: 14 bytes][sa_family: 2 bytes]
Total: 16 bytes overhead per packet
```

For `struct sockaddr_in` (IPv4):
- `sa_data` contains port (2 bytes) and IP address (4 bytes) + padding
- `sa_family` is AF_INET (2 bytes)

---

## Integration Example

**Before (standard UDP):**
```c
struct sockaddr_in server_addr;
// ... setup server_addr ...

sendto(sockfd, message, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
```

**After (proxied UDP):**
```c
#include "proxy_utils.h"

struct sockaddr_in server_addr;
// ... setup server_addr ...

send_proxy(sockfd, message, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
```

That's it. The proxy handles routing automatically.

---

## Limitations

**Hardcoded proxy address:** 127.0.0.1:5050 is compiled in. To change it, edit `proxy_utils.c` and recompile.

**Overhead:** 16 bytes added to every packet. For small messages, this is significant overhead. For larger game state packets (50-100+ bytes), it's negligible.

**Localhost only:** Designed for local testing. Using over internet would expose routing information to anyone who can intercept packets.

**No encryption:** Address data is sent in plain text. Not a concern for local testing, but would be an issue in production.

---

## Why This Design Works

The key insight: **separate where packets physically go from where senders want them to go.**

UDP routing only sees the immediate next hop. By embedding the ultimate destination in the packet data itself, we preserve the sender's intent while routing through an intermediary.

This makes the proxy:
- **Transparent** - Applications don't know it exists (except for function names)
- **Flexible** - Works with any UDP application
- **Simple** - No state tracking, no connection mapping

The tradeoff is packet overhead, but for testing network conditions locally, it's worth it.

---

**These utilities enable v1's transparent routing. See v2 for latency/packet loss additions.**
