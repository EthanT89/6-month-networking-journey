# Proxy Server

## Overview

This proxy intercepts and forwards packets between UDP endpoints, enabling simulation of real-world network conditions like __latency__ and __packet loss__. 

Integration into existing UDP applications is remarkably simple—just replace `sendto()` with `send_proxy()` and `recvfrom()` with `rec_proxy()`. That's it. Change the function names, and traffic flows through the proxy.

## The Challenge

When I started this project, I thought it would be straightforward. In theory, it is—just route all packets through a proxy that can manipulate them before forwarding. Simple enough, right?

Not with UDP.

UDP is a connectionless protocol. It sends packets to an address without confirming delivery or establishing a connection. This creates a fundamental routing problem for a proxy.

With TCP, each connection has its own socket. A proxy can create two dedicated sockets per client-server pair and map data between them seamlessly. TCP's connection-oriented nature makes this trivial.

UDP doesn't work that way. It trades reliable connections for speed and efficiency.

This left me with a critical question: how do we correctly route each packet between server and client when UDP provides no inherent connection mapping?

## The Solution

I explored numerous approaches, but most required modifying the server and client code. This frustrated me because it violated my core principle: ___integration should be effortless___.

The fundamental problem was preserving packet intent. When a sender transmits to the proxy, the packet's destination address becomes... the proxy itself. The original destination information is lost. I was going in circles.

The destination address had to be embedded in the packet data somehow. But modifying packet contents would require changes to both server and client logic—exactly what I wanted to avoid.

Then I found an elegant solution: another intermediary layer. Not another server—a pair of wrapper functions. _The sword and shield of the proxy._ 

Enter `proxy_utils.c`.

By mirroring the signatures of `sendto()` and `recvfrom()`, these wrapper functions preserve all original intent—data, destination, and source—while creating a new, enriched packet format:

1. **Sending**: `send_proxy()` prepends the destination address to the packet data and forwards everything to the proxy
2. **Proxy routing**: The proxy extracts the destination address, replaces it with the sender's address (so recipients know the source), and forwards the packet
3. **Receiving**: `rec_proxy()` unpacks the sender address and original data, seamlessly reconstructing the original packet

The result? Packets flow from server to client through the proxy completely transparently. Applications make the __same function calls__ with __slightly different names__, and everything works identically—except now we can manipulate packets in the proxy however we want.

_Latency simulation, packet statistics, stress testing—it's all possible with this approach._

## Getting Started

### Building and Starting the Proxy

The proxy reads latency and packet drop rate from `proxy_config.h` by default. For convenience, these settings can be overridden via command-line arguments, allowing quick experimentation without recompiling.

___To compile:___

```bash
gcc proxy.c ./utils/proxy_utils.c ../utils/time_custom.c ./utils/delayed_packet.c -o proxy
```

___To run:___

```bash
./proxy                  # Use defaults from proxy_config.h
./proxy 100              # Set 100ms latency
./proxy 100 5            # Set 100ms latency and 5% packet drop rate
```

**Note:** Currently, you cannot specify drop rate without latency. This could be enhanced later—it wasn't a priority for the initial implementation.

### Monitoring Statistics

Every 900ms, the proxy displays current session statistics:
- Packets received
- Packets forwarded
- Packets dropped
- Current latency (ms)

Statistics are ephemeral—they only exist for the current session. Be sure to record them before stopping the proxy if you need them later.

The `Stats` struct can easily be extended to track additional metrics as needed.

### Integration Into Your Application

If your application uses standard UDP sockets with `sendto()` and `recvfrom()`, integration is trivial:

1. Open your source file
2. Find and replace all instances of `sendto` with `send_proxy`
3. Find and replace all instances of `recvfrom` with `rec_proxy`
4. Start the proxy, then start your server and client(s)

That's it. Done.

### Important Limitations

This proxy is designed for __local testing only__. While it works beautifully on localhost, __it will not function correctly over the internet__.

Why? UDP packets may drop before reaching the proxy, and the proxy has no way to detect packets that never arrived. Statistics become meaningless, and the simulation breaks down.

For local UDP testing with `sendto()` and `recvfrom()`, however, you shouldn't encounter any issues.

## Final Thoughts

I built this proxy to stress-test the treasure-hunting game in `week3`. I'd only run it locally under ideal conditions, and I wanted to see how it behaved under real-world constraints—latency, packet loss, network congestion.

___This is the first network tool I've created___, and more are coming. I plan to use this in all my future UDP projects.

If it doesn't perfectly suit your needs or expectations, that's okay. I'm still early in my engineering journey and lack the experience to anticipate every use case. My hope is that it proves useful—for both you and me—as a practical testing tool.

Thanks for stopping by.

— Ethan