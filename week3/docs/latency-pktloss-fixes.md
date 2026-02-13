# Network Issues & Solutions

## Latency

### Problem Statement

High latency causes two critical issues in the treasure game:

1. **Input lag**: Players see their own movements only after server confirmation, making the game feel unresponsive and sluggish
2. **Stale remote player data**: Other players appear at outdated positions (their "afterimages"), making real-time interaction nearly impossible

These problems compound as latency increases—what's tolerable at 50ms becomes unplayable at 200ms+.

### Analysis

The solution is **client-side prediction**: respond to user input immediately on the client, then reconcile with authoritative server data only when corrections are needed.

This approach works because the game is purely deterministic—pressing 'W' always moves up, 'S' always moves down. The same input produces the same output (except for collisions, which block movement). This predictability allows clients to simulate movement locally with confidence.

**Key insight**: All remote player position data is already `latency` milliseconds old by the time it arrives. Clients are always seeing the past, never the present.

### Implementation Challenges

The current implementation has tightly coupled positional updates. All player coordinates are packed into a single broadcast packet sent to everyone—there's no per-player addressing. Clients identify their own position by recognizing unregistered player IDs in the update, assuming those coordinates belong to them.

This works for the current server-authoritative model but creates jittery behavior with client prediction: predicted positions get overwritten by every server broadcast, even when no correction is needed.

**Solution architecture**:
1. Clients ignore positional data that isn't tied to a specific player ID
2. Create a new packet type: **positional correction packets** (client-specific)
3. Server only sends corrections when rejecting or modifying player input
4. Clients always trust corrections from the authoritative server

### Proposed Solution: Client-Side Prediction with Collision Detection

**Client behavior**:
- React instantly to all player input and predict movement locally
- Display predicted position immediately (feels responsive)
- Only accept server corrections when explicitly sent
- Perform client-side collision detection against treasures, players, and boundaries

**Benefits**:
- Movement feels instant and responsive
- Reduces perceived latency to near-zero for the local player
- Minimizes network traffic (no need to broadcast unchanged positions)

**Remaining limitation**: Remote player "afterimage" problem persists—this is unavoidable without interpolation (which isn't suitable for discrete tile-based movement).

**Collision prediction challenge**: Clients will check collisions using data that's `latency` milliseconds out of date compared to what the server is using. This creates potential for prediction mismatch, but the authoritative server will correct any discrepancies. Currently, no infrastructure exists to store historical game state for reconciliation.

### Why Not Interpolation?

Entity interpolation was initially considered but ultimately rejected for this game:

1. **Movement is discrete, not continuous**: Players teleport between tiles instantly; there's nothing to smooth out
2. **Adds visual lag**: Interpolation delays rendering by 100-200ms, worsening the latency problem
3. **Doesn't match game feel**: Sliding between tiles looks wrong for instant tile-hop gameplay
4. **High complexity, minimal benefit**: Substantial implementation effort for a feature that degrades the experience

**Verdict**: Entity interpolation is designed for games with continuous movement (FPS, racing games). It's a poor fit for discrete, turn-based tile movement.

---

## Packet Loss

### Problem Statement

Packet loss breaks the game far more severely than expected. Testing revealed:

- Treasures appear collected on the server but remain visible on clients
- Players disappear entirely from other players' views
- Coordinate sharing bugs cause multiple players to occupy the same tile
- Critical game state becomes desynchronized

Unlike positional updates (which can be dropped safely since newer data arrives soon), certain events are **critical** and must be guaranteed delivery:

1. Treasure collection events
2. Player connection/disconnection
3. Player metadata updates (username changes, etc.)
4. Game commands

### Proposed Solution: Selective Reliable Delivery

Implement ACK/retransmission only for critical events, keeping positional updates unreliable for performance.

**Treasure collection flow**:
1. Client detects movement onto a treasure tile using `get_treasure_by_coord(x, y)` from `treasure.c`
2. Send the position update as a **reliable packet** (requires ACK)
3. Retransmit if no ACK received within timeout
4. Server validates and responds with ACK (acceptance) or correction (rejection)

**Edge case handling**: With high latency, the server may send a position correction *after* the client has collected a treasure. The server should send an ACK acknowledging receipt of the treasure collection request but rejecting the action. This requires enhanced positional correction logic on the server side (see Latency solutions above).

**Implementation notes**:
- Use `get_treasure_by_coord()` utility function to check if coordinates contain a treasure
- Keep positional updates unreliable for performance (dropped packets are acceptable)
- Critical events get ACK/retry logic for guaranteed delivery
- Server remains authoritative—client predictions can always be overruled

