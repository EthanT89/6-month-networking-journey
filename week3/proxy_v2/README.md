# Proxy Server

## Overview

This proxy aims to intercept and forward packets between two UDP servers. By intercepting these packets, we can simulate various network conditions - __latency and packet loss__. 

Integration into UDP servers is incredibly simple - `sendto()` is replaced with `send_proxy()`, and `recvfrom()` is replaced with `rec_proxy()`. Change the name, and it will flow through the proxy.

## Challenges

When first starting this, I imagined it to be quite easy - and in theory, it is. Just send all packets through the proxy and the proxy can do what it wants with the packets before sending them out.

Well, with UDP servers, this becomes a lot more challenging. UDP is an unreliable protocol, it sends packets to an address, but does not confirm if it arrived. It never makes a __connection__. 

In the TCP world, all connections get their own socket. The proxy would simply make two sockets for the server/client pair, one for the client and one for the socket. It would map between these two sockets and easily transfer data between them.

UDP does not have this functionality, not by default. It leaves that behind in turn for speed and efficiency.

So, how are we going to map each packet from the server ot the client, and vice versa?

## Solution

I brainstormed a ton of possiblities for solving the above problem. Most of these required altering some of the server and client code. I kept getting frustrated because all of these solutions violated my initial principle, ___it should be effortless to integrate into a server/client system___.

But none of these were.

Somehow, I had to extract the packet intent. Where did the sender want this packet to go? They sent it to the proxy, but since they sent it to the proxy, the destination address associated with that packet is, well, the proxy! It felt like I was going in circles.

The address needed to get packed in the message somewhere. Doing so would require altering the packet contents, which would require changes to the server/client packet logic.

In the end I decided on a solution, another middleman. Not another server, a function this time. _The sword and shield for the proxy_. Its main man.

`proxy_utils.c` contains this middleman.

It's simple, by mirroring the `sendto()` and `recvfrom()` functions, we could take all the original intent- data, destination, etc- and create an entirely new packet. This new packet had the destination address prepended to it. We would send that packet to the proxy server and it would take apart this packet and extract the destination address. Then, where it grabbed the destination address from, it replaced with the sender address. This way, the recipient knew where it came from. Finally, the proxy would forward this packet to the destination. 

The destination would then call `rec_server()`, which extracted the original address and content, painlessly extracting the original packet intent.

And with that, packets could be seamlessly sent from server to client while passing through the proxy. The server and client wouldn't notice a thing. They make the __same exact function call__, with a __slightly different name__, and it works just as before; however, now we can mess with the packets in the proxy however we want. 

_Data, statistics, stress testing, its all a possibility in this new proxy_.

## Getting it started

### Start the proxy

The proxy configurations, latency and packet drop rate, can be set in `proxy_config.h`. However, to make things more convenience, I have added command line arguments to be used with the program. This way, you only need to quickly restart the program and change the command values. If these values are not specified in the command, the default values in `proxy_config.h` are used.

`./filename --latency(ms) --droprate(%)`

___To compile the proxy server:___

`gcc proxy.c ./utils/proxy_utils.c ../utils/time_custom.c ./utils/delayed_packet.c -o proxy`

___To run:___

`./proxy`
or
`./proxy 100` If you want 100ms latency delay
or
`./proxy 100 5` If you want 100ms latency delay __and__ 5% packet drop rate

Unfortunately, there is not a way to specify ___only___ the drop rate, and not the delay. That could be added later, it was not a priority at the moment.

### Seeing Statistics

Every 900ms, 9/10 of a second, the current session's statistics are displayed. This includes packets received, packets forwarded, packets dropped, and the current latency in ms.

These statistics are only stored per-session. There is no memory or storage, so be sure to log these when you're done!

Feel free to change the `Stats` struct to track any other details.

### Integrating Into Your Program

Assuming you have a UDP server, and use `sendto()` and `recvfrom()`, this will be easy as potatoes.

In fact, you just have to press `ctrl+f`, type in 'sendto', and in the 'replace all' form, type in 'send_proxy'. Then, hit replace all. Do the same with 'recvfrom', but replace with 'rec_proxy'.

Once you've done that, you're good as gold! Just start the proxy, your server and client(s), and watch the magic happen!

###__Note:__

This works wonders locally, and is meant for local testing. __I cannot emphasize enough how poorly this will work if you use it over the internet__. Considering you are using UDP, packets may drop before/during transmission to the proxy, and not even the proxy will realize they were there. The statistics will be wrong, _it will be pointless_.

That said, if you are testing locally, and are using UDP sockets with `sendto()` and `recvfrom()`, you shouldn't have any problems!

## Final Thoughts

The goal of this was to stress test the game found in `week3`, a simple treasure hunting game over the network. I had only used it locally, and I wanted to see what it would be like with real world conditions like bad internet or congestion.

___This is the first network tool I have created___, and more are to come. I plan to use this in any future UDP projects I make.

With that, if it does not suit your needs or expectations, that's okay! I hope it can be useful to you and myself in the future, but I am just starting out my engineering journey. I lack the foresight needed for all applications.

Anywho, thanks for stopping by.

- Ethan