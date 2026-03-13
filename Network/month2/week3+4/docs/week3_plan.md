# Week 3 Plan - Latency Simulator

## Overview

The goal of this week is to build a latency simulator. To accomplish this, I will build a proxy between the server and client. All packets will get sent to this proxy as an intermediary, then they will get forwarded to the destination.

By having this proxy, we can isolate packet delay/drop simulation to a single, independent server. This proxy will be able to track all packets being sent across servers, and add variable delays, drop packets, and perform other helpful debugging tools.

## Plan

### Stage One []

First, I will implement a simple pass-through proxy. All packets will be routed to this proxy, and without making any changes to the packets, they will be immediately forwarded to the destination server.

It is important to limit the scope of functionality to making a simple proxy. Take time understanding concepts, ensuring all edge cases pass and nothing leaks through. This way, I will have a strong foundation for the more complex functionality in the next steps.

The main challenge with this will be tracking who sent what, and where they are sending it to. Further, which client sent each packet? 

The server will no longer be able to just extract the sender address, because the sender is now the proxy.

It may be necessary to limit the connection count to one client for this reason.

### Stage Two []

Next, I will add a delay queue to the server.

This delay queue will enqueue all packets received by the server, packaging them into a `delayed_packet` struct. This struct should contain the original message, time received, length, direction (`0` for server, `1` for client), destination address, destination address length, and a pointer to the next delayed packet.

A `DELAY_MS` global constant will define the time, in ms, that the packets are delayed by.

Every loop will check the timestamp of the next packet in queue, and once the `DELAY_MS` has been reached, it will forward the packet.

This allows us to simulate simple packet delays for the server and client, acting as 'lag'. By adding this feature, we can test how the program will react to having a major delay. Further, we can test our programs against real-world conditions, using statistical averages as our basis.

### Stage Three []

The last stage of this latency simulator will add packet loss simulation.

A `Stats` struct will need to be created to track the number of packets received, packets dropped, and packets forwarded.

Then, simple packet loss logic will have to be created. A global `PKT_DROP_PERC` variable will be created to adjust the percentage of packets dropped.

Every time a packet is ready to be forwarded, a random number generator will determine if that packet is dropped based on the `PKT_DROP_PERC`. All packet transmission data should be logged in `Stats`.

### Stage Four []

After the latency simulator has been fully implemented, it's time to test this simulation against our app.

I want to test different scenarios:

- 50ms latency, 0% loss
- 200ms latency, 0% loss
- 500ms latency, 0% loss
- 100ms latency, 10% loss   // representative of average connections
- 200ms latency, 30% loss   // very bad network

All findings, bugs, and other issues should be documented in a README. Any feature fixes, recommendations, and solutions should be documented as well.

### Stage Five []

Polishing the latency simulator will be the last step. Command line args, cleaner console setup, scalability and modularity.

For example, a command such as this would be handy to not have to reset the program each time we change variables:
```bash
./simulator --delay=200 --loss=10
Starting Latency Simulator Proxy...
Setup Complete!

/delay 100
/loss 5
/delay 50
...
```

### Final Documentation

Once complete with all of these stages, create a comprehensive README, documenting challenges, lessons learned, purpose, how to use, future enhancements, etc.

The proxy file itself should be documented thoroughly.