# Multiplayer Movement Server/Client

A simple UDP-based multiplayer movement demo. The server tracks connected players and broadcasts positions at a fixed tick rate. The client sends movement updates based on key presses and displays the current positions of all players.

## Features
- UDP server that manages player connections and coordinates
- Real-time position broadcasts to all clients
- Username registration and updates
- Lightweight binary packet format

## Build
From this folder:

- Server:

	gcc main.c ./utils/time_custom.c ./utils/player.c ./utils/buffer_manipulation.c -lm -o main

- Client:

	gcc client.c ./utils/buffer_manipulation.c ./utils/player.c -o client

## Run
Open two terminals.

### Terminal 1 (server)

./main

### Terminal 2 (client)

./client

You can run multiple clients in separate terminals to see multiple players.

## Controls (client)
- w / a / s / d: move up / left / down / right
- q: quit
- c: enter a command (blocking input)

## Packet Format (overview)
All packets start with a 2-byte App ID followed by a 2-byte Message ID. Additional fields follow depending on the message type.

- Update (client -> server):
	- App ID (2 bytes)
	- Update ID (2 bytes)
	- x (2 bytes)
	- y (2 bytes)

- User Update (server -> client):
	- App ID (2 bytes)
	- User Update ID (2 bytes)
	- player id (2 bytes)
	- username (up to MAXUSERNAME bytes)

- Position Broadcast (server -> client):
	- App ID (2 bytes)
	- Update ID (2 bytes)
	- Repeated per player: id (2), x (2), y (2)

## Notes
- This is a learning project focused on raw UDP and binary packing.
- The client uses non-blocking terminal input for movement updates.

