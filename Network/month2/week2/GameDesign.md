# Treasure Game Design

## Overview

This treasure hunting game presents the goal of finding and acquiring 'treasure' given a 3x3 viewport and a MxN map (variable).

Multiple users (set max) can connect to a single server. On this server, users can give simple commands for movement (WASD). Players are given directional hints as to where the treasure currently is on an x/y grid.

Treasure spawns randomly and semi-frequently. Players must reach the treasure before other players to gain points. The player who attains the set goal of points wins.

Players cannot occupy the same coordinate square as other players, walls, or other boundaries.

## Rules

Movement is restricted to the x and y coordinate system, and players can move one unit square at a time, in one direction. 

There is currently no limit on the number of actions in a given moment.

The server must verify all positional changes while the client server assumes no corrections are needed.

## UI

At all times, players will be able to see their own x,y coordinates, other players x,y coordinates, their score, and the 3x3 viewport.

### Viewport

Players can only see their coordinate square, and the squares to the left, right, above, and below. Thus, only 5 squares are visible at a time.

EX:
```
 ___
| x 0        
  0   
```
where x represents the player, 0 represents open squares, and bars represent boundaries.

There will also be indication of which direction the treasure is.
```
 ___        
| x 0    __                   __                           ___    
  0       *| (right and up)  |*  (left and up)  |* (left)   *  (up)  _*_ (down)

```

### Scoreboard
Other player's coordinates will also be displayed:

```
Coordinates:   |  Score:
_______________|_____________
YOU: (x,y)     |  3
Bobby: (x,y)   |  2
James: (x,y)   |  6

 ___ 
| x 0      *|    
  0  

```

## Implementation

### Coordinate tracking

All player coordinates are already stored. A utility function needs to be created to check the availability of a given square (coordinate). This function will first check boundaries (can be verified with static numbers), then player collisions (looping through the player list against their x,y coordinates), and lastly treasures.

If it is out of bounds, the request to move will simply be rejected.
If it will collide with another player, it will also be rejected.
If it overlaps with a treasure square, the player's score is incremented, the treasure is removed from the game state, and the player moves to that square. A new treasure is then spawned in. 

Note that all updates are always sent every tick, so the game update is the only thing that needs to be updated.

Coordinates will need to be stored for treasures, and relayed to the users every tick. May be useful to create a game state struct that contains clients, treasure counts, etc.

### Scorekeeping

Currently, players do not have a score attribute. So, this attribute must be added. Additionally, functions on both ends, server and client, will need to be updated to pack these attributes and unpack them for storage.

`player.c`: update player struct to contain point attribute (unsigned int)
`main.c`: Set initial score count. Pack score attribute into player updates. Update score count when treasure is acquired. Check when score exceeds goal.
`client.c`: Set initial score count. Unpack score attribute for players.

