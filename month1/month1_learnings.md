# Month 1 Learnings
# What I Built

## Simple TCP Server/Client

I started off week 1 with copying, line by line, the server and client code in Beej's guide. I didn't fully understand everything, and I had a lot to learn, but it was good to have some code to look at.

This short project taught me the very basics of TCP socket setup and connection through implementation. Once it was working, I deeply analyzed the code; adding comments explaining every nook and cranny of the program. If I couldn't explain something in my own words, I made sure to look deeper into it and research about it online. This was a fun and helpful first start.

I thoroughly understood the logic and concepts behind most of this code. I still could not code it myself, but I could at least understand it. The sockets were a bit confusing at first, and managing multiple clients seemed daunting.

## Upgraded Multi-client Server

Later in week 2, I created a multi-client server using `poll()`. This was a much more complex program, and required a LOT more code. I spent probably two hours dedicated to copying the code from the book to the code editor. It was the hardest day yet, because I was just sitting there and copying code that I didn't yet understand. It was frustrating and felt counter-intuitive: why I am creating something that's not my own?

I had a lot of feelings, but eventually I got through it. I ended the day off analyzing the code again. I conceptually understood most of it, but it still hadn't fully clicked yet. 

The next day I wanted to tackle it and really understand it from the ground up. So, I continued analyzing and testing the code, pushing its limits. Why was it doing all that and how? I added small custom features like welcome messages and client ids prepending their message in the chat. It was really fun and interesting to see it all come together. But I didn't take pride in it. I didn't create it. I copied it. This encouraged me to build something of my own. 

So, on the last day of the week, I created a TCP client server. I had to reference the book I was using a lot, and I ran into a lot of issues, but all the logic came from my head. I had created my first piece of network code that was (mostly) my own. It was a somewhat gratifying feeling, but I wasn't done yet. 

By the end of this small project, I had become quite comfortable with `poll()`. I understood its usage and how it allowed me to simultaneously manage multiple clients and incoming connections. 

I still am a little confused with it. Maybe that's not the right way to put it. I am not _comfortable_. That's fine for now, but I definitely want to come back and get to know it a bit better.

## Simple UDP Server and Client

On week two, I wanted to start focusing on UDP. I had learned a lot about TCP at that point, but I wanted to emphasize the difference between the two protocols and be able to distinguish the situations in which one is better than the other.

So, I started the same old system of copying Beej's code. This time it wasn't nearly as long. UDP is a lot simpler to code. This is consequence of it's nature- it is overall _simpler_. It has less overhead than TCP, and it does not check for arrival or order of the packets it sends. So, less code is needed to set it up.

I did not need to `connect()` or `listen()` for clients. All I had to do was get an address and send a message to that address, hoping it arrives. A really neat process, but I hadn't fully understood why it works. Thus, like before, I analyzed the hell out of this code. I mean a ridiculous amount analysis goes into this code sitting in front of me.

Once I finished this, I wanted to go deeper. I wanted to add custom features and logic to it, and pad it with safety precautions. The next day, I started trying to add some reliable UDP logic into the program.

The name 'Reliable UDP Server' is indicative of it's functionality- it's a more reliable version of a UDP server. Initially, I thought this was silly. Why have a reliable version of a protocol that's designed to be unreliable in nature? Don't we have TCP for that?

I thought about this intensely. Tried discovering the answer through experimentation. That didn't help. Finally, I just asked directly and looked it up. What made the purpose finally click was an example in a video game. Like I've mentioned, player positions need to be sent rapidly, and one or two dropped packets occasionally is okay, by design. However, when a player tries to shoot another play, that _needs_ to get through. That can't be dropped. So, instead of trying to switch over to TCP (which is not practical in any regard), a simple mock version of TCP is layered on top of UDP.

It goes like this: Once a packet is sent to a client, the client sends back an ACK. An ACK is just a packet that says "Hey, I received packet #15, no need to resend it." It lets the server know that a packet went through.

Now this can add a bit of overhead, so we want to only use it where necessary. So, we only send it for 'critical' events. Events like shooting or opening a loot crate.

If a server does not receive an ACK when sending a critical event, they need to resend it.

And that's that. That's what I learned about reliable UDP servers.

Then I tried to implement it. Boy oh boy did I get a run for my money. I found myself lost in terms of how to implement these concepts. I spent three days dedicated to learning C memory management- bit shifting, bytes, byte representation, buffer managment and utilization, etc. I ended up creating an entire utility file for manipulating buffers, packing (compacting) integers to these buffers, and unpacking those same numbers on the other side.

By the end of the week, I had a (very, very simple) reliable UDP server.

## (Better) Reliable UDP Server

I wanted to build something better, that actually seemed like an app. So, I dedicated week 3 to creating this. Instead of just gimmicking my way through the reliable UDP setup, I wanted to have something reliable to work with.

I started by building my own UDP server as a base. This went really, and it was my first time creating a server from the ground up all by myself. Sure, I referenced the internet occasionally, but everything was implemented by me. I was proud.

The next day, I ran into some major walls. I wanted to implement a bitmap for tracking ACKs. It basically kept track of ACK'd packets by using a starting ACK number, and a proceeding bitmap. Each n bit of this bitmap represented the n-th packet ID after the starting ACK number. THis was an extremely complex piece of functionality that took a while to understand, and even longer to implement. I spend a couple days getting this system to work.

In those following days, I also started cleaning up my client and server files. I extracted functions, utility functions, and logic into their own sections or files. I cleaned up the output formatting, and it started looking a lot cleaner.

I ended the week with a sort of reliabled UDP server. Some packets were dropped because of how the bitmap worked. The bitmap assumed that each packet id coming in would be greater than the last one received; however, if I resent a packet because it was dropped, this would not be true. It caused incorrect packets to be ACK'd and resent.

Thing is, I fully understood why this happened. I could fix it with time. But that wasn't the point of this project. I also learned how important it is to plan and think things through before implementing. I kind of just charged head on, implementing functions when needed and working my way around things. That worked before with less complex systems, but this pushed past that point, and I had some pretty annoying consequences because I did not plan.

## Custom TCP Chat Server

The goal was to create a fairly simple TCP chat server with the fundamentals I had learned so far, and to plan things out to avoid the mistakes I made while creating the Reliable UDP Server (see above). 

Overall, this was massively successful, and I even took the time to extensively document the program, along with a README informing readers how the program works, and how to add custom features! I built this in 3 days time, so about 7.5 hours in total. With about an hour of planning beforehand and an hour of documentation the day after finishing, that makes the total 9.5 hours of working on this project from top to bottom. Not bad for a guy just starting out.

I added custom functionality for handling commands and direct messaging. Instead of just broadcasting every message that comes in (I already made that before), I check for command content and direct messaging. 

The current commands include `/users`, `/myname`, and `/help`. It is very easily to implement new commands because of how I set it up. I tried to keep the mindset of keeping things modular and reusable, which allowed for an expandable program.

This experienced strengthened my understanding of the value of planning, and reinforced my confidence that I knew what was going on. I learned a lot more about sockets, connection handling, and message parsing. I understood how to better utilize pointers and custom structures (like `struct Client` and `struct LLClients`).

It wasn't just a week of reassuring myself. I expanded my palette and refined my coding ability. I learned how to plan out a program and diagram logic flow. I deepened my understanding and intimacy with the C language and data handling. I am beginning to love C, even more than I did Python.




### TCP fundamentals?

I got a strong sense of TCP fundamentals. However, there is a lot abstracted by the C language and implementation of socket.h. I would love to dive deeper sometime but I understand the core concepts enough for now.

### UDP reliability patterns?

I really enjoyed the versatility of adding reliablity on top of UDP. There was a lot of freedom with implementation and opportunities to improve runtime and speed and storage.

### Binary data and byte manipulation?
### When to use which protocol?

I now have a very clear idea of when to use each protocol. By building with each of them, I see their strengths. I see the merit in taking careful precaution and certainty with TCP, while trading reliablity for speed with UDP. I plan to make use of UDP and reliable UDP for games and physics simulations, while also using TCP for chat servers within those applications.

### What I Got Working But Don't Fully Understand

Bit shifting systems and the bitwise OR operator.

### This is important. What did AI help with that you need to revisit?

There are a couple very technical concepts that I got AI to help with. THe first was bit shifting over multiple bytes (ACK bitmap). I could not get it working and ended up needing AI to finish it and explain it to me. While I understood what it was saying, I could not recreate it. I excused it for now, but I will have to learn it at some point.

I also still have trouble with some of the buffer contents. Things like the `\r` and `\n` in the buffer and not knowing how to identify when those will be in sent messages and when they wont. I understand how to deal with them, but really its understanding how to indentify when this is happening. With telnet it gave me trouble

There are also situations with the buffers that confused me. Sometimes with the broadcasted messages and printing out contents of messages from clients there are artififacts from previous messages that linger. it confuses me but I think I fixed it by memset'ing the buffers.

### Bitmap shifting logic?

Like I mentioned about, the byte shifting over multiple bytes was confusing. Mostly because of the bitwise or operator. It probably isn't that confusing, but I haven't learned it yet, i would have at the time but I was getting frustrated learning so much already.

### Certain C pointer concepts?

Its gotten really clear how to use pointers, but there is still so much to learn. Feeling pretty good about it.

### Poll() behavior in edge cases?
### Key Lessons

### Technical (planning matters, architecture first, etc.)

My biggest takeaway (other than the multitude of actual technical content) was the importance of learning fundamentals and learning deeply. By learning fundamentals and the basics, like bit manipulation and memory management, I was able to extend this knowledge to various situations and add custom functionality. I learned that innovation stems from having a deep understanding and intuition for something, not just memorizing it. 

I have always been good at getting an intuition for something, but I have to combine intuition with understanding in order to completely harness my skill within that subject. Having the intuition passed my classes, but also caused me to have to relearn it after a few months because I didn't get deep. 

I also learned how impactful planning can be. Plans can change, but having a base idea to work off of prevents time wasted on repairs. In week 4, I didn't have to take much time fixing bugs and adjusting functions to work in different situations. Yes, that did still happen, but a fraction of the times it did in week 3. It made a huge difference and allowed me to expand off of functionality. I could think about new features without wincing at the refactoring needed. It was great.

### Personal (exhaustion vs disconnection, commitment without curiosity, using tools appropriately)

I learned a lot about myself this month. First, and maybe most importantly, I can work consistently on one thing because I want to. Not because it's a class. Not because I have practice. Because I said I was going to learn something. So, I worked every day towards that goal. It's only been a month, but that month felt amazing. I've never spent that long on something just because I wanted to. I proved to myself I could maintain strong discipline, that really, as much as I flagrantly disagreed with it, I was just lazy. 

I thought I wasn't lazy. That I had all this ambition and worked hard. But I was never working hard. That entails pushing yourself, and as much as I did successful things, I never pushed myself harder than I needed to. And that was the difference. I never felt satisfied by my accomplishments because I felt I didn't really work hard to achieve them.

I became so utterly upset with that disatisfaction, but I never did anything about it. I had new ideas all the time, things that I thought I was pushing myself to do. But after a few days, or a week, the desire to do it faded, and so did my dedication. I would feel a strong wave of determination to learn or do a certain thing. A week later? Gone. In the past.

I hated it. I had this forever rotating wheel of interests that never gave me enough time to truly do something.

So, after years and years of discontent, I thought "Well, if losing interest and motivation for these ideas is what's making me fail, how about I try and do something I don't care about?". It seemed like a silly idea. Once again, I had become infatuated with some productive idea that I felt I was determined to do. 

So I took my first steps, tried to plan it out with Claude. In the back of my mind, I thought I would fail. I didn't just think it. I thought I knew that I would fail. I felt I would stop after a week or two, or miss a day or two of sessions and lose the interest.

That interest disappeared a week ago. I'm still here, everyday, from 2-4:30.

I've never been so strict about something. Ever. I have always been laidback and would go with the flow. If I didn't have time to do my homework or research something I was interested in, so what? I could just do it the next day. But that was my weakness, I was okay with delaying it, with putting it off. Over time, this would cause ideas to lose interest. I would quit because I was lenient.

Not anymore. I won't allow myself to stop even for a day. Not because I need to learn more or love what I am learning, but because I know that will cause me to fail. I will cause myself to fail.

It's not even a thought anymore whether or not I am going to show up to that day's session. I am. There is no debate whether or not I am doing my deep work today, I am.

Suddenly, that wall that had stopped me from every mastering something had just crumbled. With just two and a half hours a day of dedication, I had completely shifted my mindset towards discipline and dedication. How funny how something so small could have such a drastic impact.

I realized just how natural my dedication to this was starting to be when my girlfriend asked to go to Oceanside for MLK Jr day. We had that monday off, so we didn't have school. Immediately, I knew I was going to have to get up early to do my deep work, and change up the time for that day. I never, not even for a second, thought "Well its a holiday so I will just take the day off". I knew I was going to do it, so I had to do it earlier in the day. My girlfriend was surprised by this. She thought I was just going to skip that day. I don't blame her, I would probably do that with most things. It makes sense. But that small moment made me realized how the small changes had accumulated into a massive change of character.

It's up to me to stay in character. I see this person that works hard, and I will continue to be him.

After four successful weeks, I know what it takes to keep going. And I will stop at nothing to do so.

Honestly, I am loving this. I feel happier, more satisfied. I've noticed that I have started to take my time, to appreciate it more. Intentionality makes a huge difference. Suddenly, with this intense, packed schedule, I feel happier. I enjoy my time more. Things feel more meaningful. What I thought I was missing was there all along. I didn't need to do something, I needed to think differently. I am happy I started this. I truly feel this idea is something I want to implement my whole life. To dedicate some time every day towards doing something. Learning, researching, building, whatever it may be.

I also realized how important building habits is. By coming here everyday, which seemed like a big deal at the start of this journey, it eventually became normal. A part of my routine. I have read Atomic Habits before and my friend talked to me about the concepts, but having done this as part of my own independent journey really nailed in the concepts. It makes a huge difference. Its the same way as in how I hated brushing my teeth as a kid and it seemed like such a huge amount of time and I would alwayssss forget, but as I grew older and did it more consistently, it just became part of my daily routine. I never miss a brush because it feels so normal. Its hard to forget because its in my routine. Same with face washing, showering, watching youtube while I eat my food. Its routine. No one 'needs' to take their phone to the bathroom with them, but its part of their routine and oftentimes it feels wrong or different to do otherwise. It becomes the norm, and psychologically, that makes a huge difference. So drastic I would call it the most important part of this whole journey.

Further, it has taught me to be more intentional with my time. I plan things out more often. Not rigidly, but by just having a rough plan of what to do, I suddenly always have something to do. I don't get distracted as often. By dedicating time to something, and nothing else during that time, I am much more effective at it. For example, If worked on an assignment for an hour while also watching youtube in between questions or during it, I would get a substantially larger amount of work done If I were to spend just 30 minutes wholly dedicated to the assignment and then spending the 30 minutes of watching youtube after.

There are times to multitask. I will fold socks just as fast watching a show as I would dedicating the time to just folding. But 9/10 times I will be substantially more effective if I am dedicating my time towards one thing specifically. I strongly believe this applies to everyone.

I think I was always worried about not getting time to watch youtube or shows or instagram or anything like that, so I would do it during  my work, or take frequent breaks. Now, I often only do one thing at a time.

It's more enjoyable this way too. By being entirely focused and immersed in what I am doing, I learn more, do more, feel more, and enjoy it more. Its just that much better.

I also worried so much about people getting upset or feeling hurt by my lack of response or not responding quickly. So, I would always text back or respond as soon as I could. Now, I will text back when I am done. It really isn't urgent. I sure hope I am not offending anyone, but I take my time replying to people now.

I think I make up for this lack of communication by now being immersed in every face to face conversation I have. You will hardly ever catch me on my phone while I am with someone. If I am with someone, I am hanging out with them, not going on my phone.

All this has been affected by my journey.

It doesn't seem direct, but it is. I learned a lot from this, during and after my sessions.

I find myself always walking to class, in no rush, no music or headphones, just enjoying the walk. I just don't rush as much anymore, and funny thing, I am exponentially busier. What a paradox.

By filling my time up with intentional and fulfilling things, I have become so much happier. I joined the rowing team and now I have a community I can connect with every day. I get to get out and be active, and that pays off immensely.

I took a huge leap by doing all this stuff, but more and more, I feel myself never wanting to go back.

With all this in mind, I am very careful about not overloading myself. I try and make sure I always have freetime in every day. That at some point, I am sitting in my room, and relaxing. Doing whatever it is I want, videogames, hanging out with someone, reading, watching something. The few hours I dedicate to this feel more impactful then the half-days poured into content consumption and relaxing I had before. This intentionality and 'earned' freetime means so much more than the guilty time spent doing nothing.

Suddenly, I am no longer doing 'nothing'. I am relaxing. I am having a slow evening. I am reading. I am watching my favorite creator on YouTube. I am not guilty of it. It feels deserved. I am no longer shy about who I am and what I am doing, because I am becoming proud of all of that.

So, maybe the biggest thing I am getting out of this is confidence. In who I am. What I am doing.

I enjoy my time now. I don't worry very often if I am doing the right thing. If I need to be doing more. I am so much more confident and that much happier.

I will keep doing this.

Side note for Claude: I am not perfect at any of this yet. I am still not amazing at being intentional with my time and all the stuff I mentioned isn't perfect yet. But I am scratching the surface

## Month 2 Questions
### What do you want to understand next?
I want to understand Client prediction and state synchronization. I am extremely interested in it's application, specifically in programs with physics, like videogames.

#### State synchronization?
#### Client prediction?
#### Lag compensation?
#### Building actual tools?