### Week 3 Reflection

## It was a very technically difficult week

This week, I attempted to make something genuinely complex, and make it work well. I decided to create a full reliable UDP server with what I knew. Turns out I didn't know half as much as I thought I did. 

It was mainly C concepts that made me struggle. Specifically the bitmap I had to implement, as well as managing outgoing packets with custom data structures. Data structures have been hard for me because I am so used to Python and C++, which offer a much more straightforward and simpler way to manage data.

C gives me the flexbility, but also presents me with a much bigger wall to climb. I'm climbing, but I am not nearly over it.

I am happy with how well I understand the underlying protocols and systems, however. It was hard implementing this system, but every bit of it made sense.

In that regard, the hardest part was actually making it. Creating multiple functions, libraries, and utilizing pre-exiting data structures and functions to work towards the goal of reliable UDP was insanely difficult.

I learned how important it is to plan ahead and design the app before actually building it. Before I knew it, I had intertwining logic, unnecessary functions, and brute force methods that could have all otherwise been avoided if I had planned this beforehand. But thats okay, because I did not even know enough beforehand to plan all of this. So I'll give myself a pass.

## Day by day breakdown (broad overview)

# Week 3 - What Actually Happened

# Monday (Day 11)
- Read Gaffer's virtual connections article
- Built UDP server from memory (99%)
- Debugged without AI
- Felt confident

# Tuesday (Day 12)
- Read reliability article
- Hit strlen(buf) bug with binary data
- Built heartbeat system (100ms intervals)
- Starting to see the bigger picture

# Wednesday (Day 13)
- Got paralyzed by data structure choice
- Realized I needed to just pick one and build
- Started ACK implementation

# Thursday (Day 14)
- Built entire bitmap ACK system
- Hit so many C walls
- Used AI strategically when stuck
- Learned: pointers, bitwise ops, memory layout
- Almost felt like giving up around 3pm
- Kept going

# Friday (Day 15)
- Implemented timeout/retransmission
- System actually works
- Packets drop, retransmit, eventually arrive
- Built something complex and understood it

## What I Actually Built
- 32-packet sliding window
- Bitmap-based ACK tracking  
- Automatic retransmission
- Packet loss simulation
- Full client/server protocol

## The Real Lesson
Not about UDP. About pushing through complexity.
About using AI as a tool, not a crutch.
About finishing what you start.