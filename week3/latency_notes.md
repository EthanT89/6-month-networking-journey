# Latency Testing Notes

## The Experiment

The goal of this experiment is to add simulated latency and packet dropping. This simulates real world circumstances such as poor internet, slow server speeds, etc.

By implementing these simulated network issues, we can stress test various applications. For this experiment, we will only be testing the multiplayer treasure hunt game. All prior testing of this game has been done locally, with optimal network latency (near-perfect on LAN)

## Initial thoughts

Going into this, I felt there would be a few simple issues - obvious lag in player movement, lots of unnecessary jittering and server correction, and inaccurate treasure collection/player detection.

This was a pretty optimistic ideal in retrospect.


## First Test - Latency

First, I wanted to see the effects of adding a bit of lag. All packets would be delayed by a certain figure in milliseconds, max 999ms.

The immediate consequences were visible, a substantial lag as the player tried to move around, but their movements were seen moments later, rather than instantaneously.

Right away, I could see the important of client prediction. I added only a 50 ms delay, and it made the gameplay pretty frustrating. 50ms is a pretty good average. I then tested 100ms, it was near unplayable. 100ms is a pretty common latency, so this stook out as a major issue. 

As I tested beyond that latency, it was unbearable. I could not predict where I was going, and it was hard to manage my position. It was hard to race players or follow them, because I was only seeing their afterimage.

### Take Away

I need to add client prediction, interpolation, and smoother correction tactics. It might be a good idea to add client-side verfication, especially for cases like running into players, boundaries, and exceeding movement paces. This way, a lot of the correction jitters can be eliminated off the bat.

Further, the client's movements should be immediately displayed. When I move up, I want to see myself move up immediately. The server can correct that, but as a user, I would rather it be smooth most the time, rather than seeing a constant delay with minimal jitters.

## Second Test - Packet Loss

I wanted to simulate packet loss, a common, yet confusing, issue gamers run into. It seemed so mysterious, where are my packets?

Right off the bat, I realized that this was a much more serious issue than the latency. Latency made it annoying to play, but all game logic still worked correctly. Everything was the same, I was just seeing in the past.

With packet loss, the game was fundamentally changed. I would try to move, and instead of moving, I would stay still- my position update was never sent to the server.

I would try to collect a treasure, and move to it's location. But I didn't see my score go up. Then, I moved away, and the treasure was still there! I tried collecting it over and over again but it didn't work. Some treasures still worked though, most infact.

This was caused by the server receiving my position and recognizing I collected a treasure, but when it tried to send me the packet acknowledging this and removing the treasure, it dropped. So, my client never knew to increase my score and remove that treasure. The next times I tried to collect it, well, the server thought there wasn't even a treasure there- to its knowledge, I already collected it.

Even worse, with critical updates (user connections, initial connections, treasure collecting, etc), the game would break if one of these were lost. 

I tried to connect to the server, and on the client side it said it worked, but the server never got a connection message, so the client console was just sitting there, frozen.

I tried again, this time it worked, but I didn't receive the user updates. There was another user in the game, but I never received their info, so my client had no record of them. Suddenly, I my player kept teleporting from one coordinate to a completely different, seemingly unrelated, coordinate. I realized it was the other player's coordinate. Since I didn't have their data, the client server interpreted their data as mine, and kept putting me in their position.

So, when a player's info is missed, I don't even know they are there, and the client server breaks, not understanding how to deal with this.

## Third Test - Latency + Packet Loss

With both of these combined, it was an unbearable experience. I couldn't 'feel' my movement, and half of it was just missing. 

I could see other players, but they seemed to be teleporting around.

I couldn't collect most treasures, they would break the same way I mentioned earlier. I didn't even realize another player existed because of the same issue above.

It was an awful experience, and I realize how poorly architected my program is for these scenarios. With a small delay, it stopped feeling real time. With some packet loss, the game fundamentally broke. 

Reflection soon to come, but I have to log off for today.