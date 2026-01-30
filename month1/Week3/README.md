# Reliable UDP Protocol Implementation

Built Week 3 of my 6-month networking journey.

## What This Does

Implements reliable delivery on top of unreliable UDP using:
- **Sliding window ACKs** (32-packet bitmap)
- **Automatic retransmission** (1 second timeout)
- **Packet loss simulation** (configurable drop rate)

## How It Works

**Client:**
1. Sends packet with protocol_id + packet_id + message
2. Tracks sent packets with timestamp
3. Receives ACK bitmaps from server
4. Retransmits if no ACK after 1 second

**Server:**
1. Receives packets
2. Maintains 32-packet ACK bitmap
3. Sends back: most recent packet_id + bitmap of last 32 packets

**ACK Bitmap:**
- Bit 0 = most recent packet
- Bit 1 = (most_recent - 1)
- Bit 31 = (most_recent - 31)

## Testing

Set `PACKET_DROP_RATE` to 30-50% and watch retransmissions work.

## What I Learned

- Bit manipulation in C (shifting, masking, byte boundaries)
- Time precision with clock_gettime()
- Binary protocol design
- How TCP actually works under the hood