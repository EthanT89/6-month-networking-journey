# Server Architecture Checklist

### Utils
- [x] Create Player Utils
    - [x] Player and Players structs
    - [x] Add Player
    - [x] Remove Player
    - [x] Find Player

- [x] Create Time Utils (tick management)
    - Get current time
    - Compare a time period to an interval value
    - Compare a past time to the current time given an interval value

### Server Init

- [ ] Create Server Socket
- [ ] Receive a Message
    - [ ] Unpack Message
    - [ ] Verify Using APPID
    - [ ] Determine Message Type
    - [ ] Create New Player Struct (if new connection)
    - [ ] Handle content (covered in data section)
- [ ] Handle Disconnects
- [ ] Close Socket On Quit


### Handle Data

__After Unpacking, Verifying, accepting new connections__

- [ ] Map address to player
- [ ] (future) Handle Commands (`/help`, `/changename`, etc.)
- [ ] Update Player Coords and other metadata


### Game Loop

