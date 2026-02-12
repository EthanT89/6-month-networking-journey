# Proxy v2 Test Applications

Simple test programs for validating the proxy's latency simulation and packet loss features.

---

## Test Programs

### Basic Communication Tests

**`test_server.c`** - Simple UDP server that uses `rec_proxy()`
- Listens on port 6000
- Receives messages through the proxy
- Prints sender address and message content
- Useful for verifying transparent routing works

**`test_client.c`** - Simple UDP client that uses `send_proxy()`
- Sends messages to test_server through proxy
- Interactive: type messages to send
- Displays sent byte count
- Useful for manual testing of proxy routing

### Packet Loss Testing

**`test_delayed_packet.c`** - Unit tests for the delayed packet queue
- Tests `enqueue_packet()`, `pop_packet()`, and `ready_to_send()`
- Validates timing logic and wraparound handling
- Ensures queue ordering (FIFO)
- Run before making changes to delayed_packet.c

---

## Running the Tests

### Basic Communication Test

Verifies that packets route correctly through the proxy with the wrapper functions.

**Terminal 1 - Start proxy:**
```bash
cd week3/proxy_v2
gcc proxy.c ./utils/proxy_utils.c ../utils/time_custom.c ./utils/delayed_packet.c -o proxy
./proxy 50 5  # 50ms latency, 5% loss
```

**Terminal 2 - Start test server:**
```bash
cd week3/proxy_v2/tests
gcc test_server.c ../utils/proxy_utils.c -o test_server
./test_server
```

**Terminal 3 - Start test client:**
```bash
cd week3/proxy_v2/tests
gcc test_client.c ../utils/proxy_utils.c -o test_client
./test_client
```

Type messages in the client terminal. They should appear in the server terminal after ~50ms delay (plus any dropped due to 5% loss).

---

## What to Look For

### Successful Routing
- Client shows: "Sent X bytes through proxy"
- Proxy shows: "Received data. Routing..."
- Server shows: "Received X bytes from 127.0.0.1:YYYY"
- Message content appears correctly

### Latency Simulation
- With 100ms delay, messages arrive ~100ms after sending
- Watch proxy statistics—every 900ms it prints packet counts
- "Packets Received" should increment as client sends
- "Packets Forwarded" should increment ~100ms later

### Packet Loss
- With 10% loss, ~1 in 10 messages won't reach server
- Proxy statistics show "Packets Dropped"
- Client sends message but server never receives it
- No error messages—this is expected UDP behavior

---

## Building Individual Tests

**Test server:**
```bash
gcc test_server.c ../utils/proxy_utils.c -o test_server
```

**Test client:**
```bash
gcc test_client.c ../utils/proxy_utils.c -o test_client
```

**Delayed packet unit test:**
```bash
gcc test_delayed_packet.c ../utils/delayed_packet.c ../../utils/time_custom.c -lm -o test_delayed_packet
./test_delayed_packet
```

---

## Test Code Notes

These test programs were intentionally kept simple—they're minimal UDP applications that demonstrate the proxy works. They're not production-quality code.

**What they test:**
- Transparent routing (send_proxy/rec_proxy work)
- Address preservation (sender info correct)
- Bidirectional communication
- Latency simulation (packets delayed correctly)
- Packet loss (some packets don't arrive)

**What they don't test:**
- Multiple concurrent clients (can be tested by running multiple test_client instances)
- Large packet sizes (game packets are ~50-100 bytes)
- Extreme network conditions (1000ms+ latency, 50%+ loss)
- Edge cases (empty packets, invalid addresses, etc.)

For comprehensive testing, integrate the proxy with the treasure hunt game (see parent README).

---

## Troubleshooting

**"Address already in use"**
- Test server still running in background
- Wait 30 seconds or run: `killall test_server`

**"Connection refused"**
- Start proxy first, then server, then client
- Proxy must be listening on port 5050

**Client sends but server never receives:**
- Check proxy is running
- Verify test_server shows "listening on port 6000"
- If packet loss is 100%, all packets drop (reduce drop rate)

**Messages arrive instantly despite delay:**
- Check proxy command-line args: `./proxy 100` for 100ms
- Verify proxy statistics show "Latency: 100ms"
- Make sure you're running the correct proxy binary (recompile if needed)

---

**Status: Tests validate proxy v2 features. For real-world testing, use with actual game applications.**
