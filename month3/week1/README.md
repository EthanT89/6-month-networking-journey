# Week 9: Distributed Task Queue System

**Days 41-45 | February 24-28, 2026**

Month 3. New territory. After spending two months on multiplayer game servers, this week was about building infrastructure—the kind of system that runs in the background at every tech company. Task queues, job processing, worker coordination. Stuff I've used (GitHub Actions, image processing pipelines) but never built.

The goal: clients submit jobs, a server manages them, workers execute them. Sounds simple. It's not.

---

## What This Does

Three pieces:

**Server** - Listens on two ports. Clients connect to submit jobs (port 1209). Workers connect to receive jobs (port 1205). Server manages the queue, tracks job state, assigns work when workers are free.

**Workers** - Standalone processes. Connect to server, get an ID, wait for jobs. When assigned work, they execute it (word counting, text manipulation, etc.) and send results back. Then wait for the next job.

**Client** - Submit jobs, check status, get results. One-shot connections—connect, send command, get response, disconnect.

Right now, job types are simple:
- `wordcount` - Actually counts words now (fixed a bug where it counted characters)
- `charcount` - Counts non-space characters
- `echo` - Returns whatever you send
- `capitalize` - MAKES TEXT LOUD

Workers sleep for 5 seconds per job to simulate real processing. Makes it easier to watch the queue system work—see jobs pile up, workers get assigned, queue drains. Next week's jobs will actually do real work (file processing).

---

## The Journey (What Actually Happened)

### Day 41-42: Protocol Design & Server Foundation

Monday and Tuesday were setup. Protocol design, server skeleton, getting connections working.

Used TCP instead of UDP because jobs need to be reliable. I spent two months in Month 2 building reliability on top of UDP (ACKs, retransmission, all that). Not doing that again when TCP solves it for free.

Binary protocol like the game servers. APPID (4379—random number) + message type (2 bytes) + data. Smaller than JSON, faster to parse. Already had the packing functions from Month 1.

Biggest new thing: epoll. I'd used `poll()` before, but epoll is what real servers use on Linux. Single-threaded, handles thousands of connections. Pattern is: create epoll instance, register file descriptors, block on `epoll_wait()`, handle events when they're ready. Way more efficient than forking a process per connection.

The server struct tracks everything:
```c
struct Server {
    int epoll_fd;
    int worker_listener;      // Port 1205 (workers connect here)
    int client_listener;      // Port 1209 (clients connect here)
    int job_id_ct;
    struct Stats *stats;      // Real-time metrics
    struct JobQueue *queue;   // FIFO job queue
    struct Jobs *jobs;        // All job records
    struct Workers *workers;  // Connected workers
};
```

### Day 43: Job Processing & Worker Implementation

Wednesday: making workers actually do something.

Workers connect to port 1205. Server sends them an ID. They sit in an event loop waiting for job packets. When a WPACKET_NEWJOB arrives, they parse the metadata, figure out the job type, execute it, send results back.

Had to think through error cases. What if someone submits "asdfasdf one two three" as a job type? Worker needs to reject it cleanly. Added error codes: WERR_INVALIDJOB for unknown job types, WERR_UNKNOWN for anything else.

Job type parsing was trickier than expected. Metadata comes as "wordcount hello world test". Need to extract "wordcount", null-terminate it, check against known types, then move the rest ("hello world test") to the start of the buffer for processing. Lots of pointer arithmetic. Got it wrong the first time—forgot to null-terminate and `strcmp()` was reading garbage.

Once the job type is known, routing is simple. Switch statement: JTYPE_WORDCOUNT calls `job_wordcount()`, JTYPE_ECHO calls `job_echo()`, etc.

### Day 44-45: Load Testing & Chaos

Thursday the system worked. Friday I wanted to break it.

Built `submit_jobs.c`—takes a number N, submits N jobs as fast as possible. Opens connection, sends job, gets response, closes, repeat. Lets you fill the queue instantly.

Built `create_workers.c`—forks N worker processes using `execl()`. Want 50 workers? Done. Want 500? Also done (narrator: it was not done).

**The Friday afternoon experiment:**

First test: 1000 jobs, 5 workers. Perfect. Queue grew to 995, workers chewed through them steadily. Stats command showed it all happening in real-time. This is when it felt real—watching a distributed system actually work.

Second test: 80 jobs, 1500 workers. Disaster. My laptop couldn't fork 1500 processes. Hit `ulimit -u` (max user processes). System froze. Couldn't even run `ls`. Had to `pkill -9` from an existing terminal session. Learned: don't spawn unlimited processes on a laptop.

Third test: 50 jobs, 10 workers. Sweet spot. Fast processing, no crashes.

Added a `stats` command to the server so I could watch this live. Type "stats" in the server terminal, see jobs processed, success rate, active workers, queue depth. Made debugging way easier.

---

## How It Works

### Communication Flow

**Submitting a job:**
- Client connects to server on port 1209
- Sends: [APPID:2 bytes][JOBSUBMITID:2 bytes][job metadata]
- Server creates job, adds to queue, returns job ID
- Closes connection

Server doesn't keep client connections open. One request, one response, done. Keeps it simple.

**Workers connecting:**
- Worker connects to server on port 1205
- Server assigns it an ID: [APPID:2][WPACKET_CONNECTED:2][worker_id:2]
- Connection stays open (workers are long-lived)

**Assigning jobs:**
- Server checks queue when a worker becomes available
- Pops job from queue (FIFO)
- Sends to worker: [APPID:2][WPACKET_NEWJOB:2][job_id:2][metadata]
- Worker status changes from W_READY to W_BUSY

**Getting results:**
- Worker finishes, sends back: [APPID:2][WPACKET_RESULTS:2][result string]
- Or fails: [APPID:2][WPACKET_STATUS:2][W_FAILURE:2][error_code:2]
- Server updates job status, marks worker ready for next job

### Job States
- `J_IN_QUEUE` (0) - Waiting for worker
- `J_IN_PROGRESS` (2) - Assigned to worker
- `J_SUCCESS` (1) - Completed successfully
- `J_FAILURE` (3) - Failed after retries

Jobs can be retried up to 3 times before permanent failure.

### Worker States
- `W_READY` (0) - Available for work
- `W_BUSY` (2) - Processing job
- `W_SUCCESS` (1) - Job completed (transitions back to READY)
- `W_FAILURE` (-1) - Job failed (transitions back to READY)

---

## Build & Run

### Prerequisites
- GCC compiler
- Linux/Unix environment (uses epoll for I/O multiplexing)
- No external dependencies

### Compile

From the `month3/week1/` directory:

**Server:**
```bash
gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c -o server
```

**Worker:**
```bash
gcc worker.c ./utils/buffer_manipulation.c ./utils/job_processing.c -o worker
```

**Client:**
```bash
gcc client.c ./utils/buffer_manipulation.c -o client
```

**Load Testing Utilities:**
```bash
gcc submit_jobs.c ./utils/buffer_manipulation.c -o submit_jobs
gcc create_workers.c -o create_workers
```

### Run

**Terminal 1 (Server):**
```bash
./server
```
Server listens on port 1209 (clients) and 1205 (workers). Type `stats` to see statistics. Type `quit` to shutdown gracefully.

**Terminal 2+ (Workers):**
```bash
./worker
# Or spawn multiple:
./create_workers 10
```
Each worker connects, receives an ID, and waits for jobs.

**Terminal N (Client - Submit Job):**
```bash
./client submit "wordcount hello world how are you"
# Returns: Job ID: 0
```

**Terminal N (Client - Check Status):**
```bash
./client status 0
# Returns: Job in progress. Worker: 1234
# Or: Job complete. see 'results' for more info.
```

**Terminal N (Client - Get Results):**
```bash
./client results 0
# Returns: word count: 5
```

**Terminal N (Load Testing):**
```bash
# Submit 100 jobs quickly
./submit_jobs 100

# In another terminal, spawn 10 workers to process them
./create_workers 10
```

---

## What I Learned

### Epoll is Magic

Before this, I'd used `poll()` (Week 1 chat server). Epoll is the Linux version, way more efficient.

Pattern: create epoll instance, register every file descriptor you care about, call `epoll_wait()` which blocks until something happens. When it returns, you get a list of file descriptors that are ready to read/write. Handle them, go back to waiting.

Single-threaded. No forking. Handles hundreds of connections easily. The server ran 1000+ workers without threads or child processes. Just one event loop checking "is there data to read?" for all the sockets.

This is how nginx works. How Redis works. Event-driven architecture. Way more efficient than spawning threads.

### Binary Protocols Are a Pain to Debug

JSON would've been easier. But binary is faster and smaller, so I committed.

Every packet: APPID (2 bytes) + message type (2 bytes) + data. The `packi16()`/`unpacki16()` helpers from Month 1 handle byte order. Works across different architectures.

Debugging though? Brutal. Can't just print the packet and see what's wrong. I added debug statements everywhere showing byte counts: "sent 51 bytes", "job metadata: 47 bytes". If the numbers don't match, something's wrong with the packing.

One bug took an hour: I was using `strlen()` on binary data. Strlen stops at null bytes. The buffer had data past the null. Switched to tracking lengths explicitly.

### State Management Gets Messy Fast

Jobs have state. Workers have state. The server tracks both. Keeping it all consistent is where bugs hide.

Worker disconnects mid-job? Need to detect that (read() returns 0), find their current job, re-queue it, remove the worker from the list.

Job fails? Increment retry counter. If it's failed 3 times, mark it permanently failed. Otherwise re-queue.

Worker reports success? Update job status, increment stats, mark worker as ready for more work.

I missed edge cases. Like: what if a worker takes so long the server considers it dead and reassigns the job, then the original worker sends success? Solution: check that the job's worker_id matches before accepting results. If not, ignore it.

This is why distributed systems are hard. State everywhere, no global clock, things fail unpredictably.

### System Limits Are Real

Friday's 1500-worker test taught me about `ulimit -u`. My laptop's process limit is ~127,000, but something about forking that many workers at once killed it. Couldn't run any commands. Terminal was frozen.

Had one terminal still open from earlier. Ran `pkill -9 client` and `pkill -9 worker`. Took a minute but eventually cleaned up.

Lesson: test your limits before hitting them in production. Also: forking 1500 local processes is stupid. In real systems, you'd have workers on separate machines or use thread pools. Or you'd use something like Kubernetes to manage worker processes.

But breaking things is how you learn where the boundaries are.

### Workers Block on Jobs (Need to Fix This)

Right now, workers run jobs synchronously. `sleep(5)` to simulate processing. If a job hangs, the worker hangs.

In production, you'd fork a child process for the job. Parent worker stays responsive, can accept cancellation requests. Child does the actual work, reports back when done.

Or even better: use containers. Each job runs in a Docker container, isolated, with resource limits. Worker just manages the container lifecycle.

That's next week's problem. This week was about getting the basic architecture working.

---

## Known Issues

### No Persistence
Everything's in memory. Server crashes? All job state is gone. Need to add SQLite or flat file storage. That's coming.

### No Cancellation
Once a job starts, it runs to completion. Can't cancel it. Would need to add cancellation packets and make workers check for them periodically.

### Workers Die Silently
If a worker crashes, the server eventually notices (read() returns 0). But there's no graceful shutdown protocol. Workers should send "I'm shutting down" before disconnecting.

### No Load Balancing
First available worker gets the job. No consideration of worker capabilities, current load, or job complexity. A "resize 10GB image" job gets treated the same as "echo hello".

### Results in Memory
Job results live in fixed buffers (MAXRESULTSIZE). Large results get truncated. Should stream to disk or database.

### Zero Security
Anyone can connect. No authentication, no encryption, nothing. In production, you'd need API keys, TLS, maybe OAuth.

### No Job Priority
FIFO queue. Every job is equal. Should have priority levels—critical jobs jump the queue.

All fixable. But for week 1, the goal was getting the core working. These are week 2+ problems.

---

## Files & Structure

### Main Programs
- `server.c` (540 lines) - Job queue server with epoll-based event loop
- `worker.c` (244 lines) - Worker process that executes jobs
- `client.c` (142 lines) - CLI for job submission and status queries
- `submit_jobs.c` (113 lines) - Load testing utility for rapid job submission
- `create_workers.c` (51 lines) - Spawns multiple worker processes

### Utilities
- `utils/jobs.c/h` - Job struct management (add, remove, lookup by ID)
- `utils/workers.c/h` - Worker struct management (same pattern as jobs)
- `utils/job_queue.c/h` - FIFO queue implementation with create/add/pop
- `utils/job_processing.c/h` - Job execution logic (determine type, route to handler)
- `utils/buffer_manipulation.c/h` - Binary packing/unpacking (reused from Month 2)
- `utils/time_custom.c/h` - Timing utilities (reused from Month 2)

### Configuration
- `common.h` - Shared constants (ports, buffer sizes, packet IDs, job types, status codes)

**Total: ~1400 lines of new code, plus ~400 lines of reused utilities**

---

## What's Next

**Week 10 (next week):**

The plan is file-based jobs. Jobs that actually do something useful:
- Image resizing (upload an image, get back a thumbnail)
- File compression (tar/gzip)
- Document conversion (markdown to PDF, etc.)

This requires building a file transfer system. Can't just send file paths—workers might be on different machines. Need to send the actual file data, probably in chunks. Then workers save it, process it, send results back (which might also be a file).

Also: multithreading for complex jobs. Right now, workers handle one job at a time. Simple jobs are fine, but file processing can take minutes. Want workers to handle multiple jobs concurrently. That means pthreads, mutexes, thread-safe data structures. All the things that make concurrent programming interesting (read: painful).

**Later weeks:**
- Job persistence (survive server crashes)
- Distributed workers across machines
- Better retry logic and job priorities
- Eventually: deploy to actual cloud instances and stress test at scale

But first: make jobs actually useful.

---

## Reflections

This week felt different from Month 1 and 2. Game servers are immediate—see the players move, collect treasures, everything's visual. Task queues are invisible. The interesting part is watching the system handle load, seeing jobs flow through the queue, monitoring stats.

Friday's load testing was the highlight. Watching 1000 jobs move through the system, queue growing and shrinking, success rate staying at 100%. That's when it clicked: this is how real infrastructure works. Redis, RabbitMQ, AWS SQS—they're all doing variations of this.

Building it from scratch (no frameworks, just sockets and C) teaches you what the abstractions are hiding. Now when I see "job queue" in a system design doc, I know what that means: state management, worker coordination, failure handling, retry logic. All the stuff that makes distributed systems hard.

The code is rough. No persistence, no authentication, edge cases everywhere. But it works. Jobs get processed, workers stay busy, the system scales (until you spawn 1500 processes and crash your laptop).

Next week's file processing will be harder. Can't just send text—have to transfer actual files, probably in chunks. And multithreading adds a whole new class of bugs (race conditions, deadlocks, all the fun stuff).

But that's the point. Build something simple first, then make it complex. Month 1 was sockets. Month 2 was real-time updates. Month 3 is infrastructure. Each month builds on the last.

---

**Week 9 complete. Basic task queue working. Load tested with 1000 jobs. Next: file processing and multithreading.**