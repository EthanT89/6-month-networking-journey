# Week 5: Simple Multiplayer Movement Server

## Goal
2-4 clients can connect, each controls a player (x,y coordinates), 
all see each other move in real-time

## Server Architecture

### State
- Player list: {id, x, y, last_input_time}
- Game loop: 60Hz tick rate

### Core Loop
1. Accept new connections
2. Receive input from clients (move commands) - Also handle disconnects
3. Update positions (30 pixels/sec or whatever) - Logic check to avoid cheating.
4. Broadcast state to all clients (every tick)

### Messages
- CLIENT → SERVER: {input: 'up'/'down'/'left'/'right'}
- SERVER → CLIENT: {players: [{id, x, y}, {id, x, y}, ...]}

## Client Architecture

### State
- My player ID
- All player positions

### Core Loop
1. Read keyboard input
2. Send to server
3. Receive state updates
4. Render positions

## Day Breakdown
- Mon: Read Gabriel Gambetta, plan detailed architecture
- Tue: Server tick loop + basic state
- Wed: Client + server communication
- Thu: Multiple clients, polish movement
- Fri: Test, document, maybe add simple interpolation

## Success Criteria
- 3 clients connected
- All see each other move smoothly
- No major lag/jitter
- Clean architecture (planned first)