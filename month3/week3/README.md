# Week 10: File-Based Task Queue

**Days 46-49 | March 2-5, 2026**

Taking the task queue from Week 9 and making it actually useful. Text-based jobs (wordcount, capitalize) are fine for testing, but real infrastructure processes files: images, documents, videos. This week was about building file transfer into the system and making workers handle real file operations.

Turns out file transfer over TCP is way harder than I expected.

---

## What Changed From Week 9

**Week 9 system:**
- Client sends job metadata as text: `"wordcount hello world test"`
- Server stores it in memory
- Worker receives text, processes it, sends results back as text
- Everything fits in fixed buffers

**Week 10 system:**
- Client uploads a file
- Server stores file in `server_storage/`
- Server sends file to worker
- Worker saves to `worker_storage/worker-<id>/content.txt`
- Worker processes file (can be any size), writes results to `results.txt`
- Worker sends results file back to server
- Server stores in `server_storage/job-<id>.txt`
- Client retrieves results file, saves to `client_storage/results.txt`

Files move through the system. Workers never see the client. Server is the hub.

---

## What This Does

Same three components as Week 9, but everything's file-based now:

**Client** - Three commands:
- `./client submit <jobtype> <filepath>` - Upload file, specify job type (wordcount, charcount, capitalize, echo)
- `./client status <job_id>` - Check if job is queued, in progress, or complete
- `./client results <job_id>` - Download results file to `./client_storage/results.txt`

**Server** - Dual-port architecture:
- Port 1209: Accepts client requests (job submission, status queries)
- Port 1205: Manages worker pool (assigns jobs, receives results)
- Stores all files in `./server_storage/` (job files indexed by job_id)
- Epoll-based event loop handles hundreds of connections
- Type `stats` to see metrics, `queue` to see pending jobs, `quit` to shutdown

**Workers** - File processors:
- Connect to server, receive unique ID
- Each worker gets private directory: `./worker_storage/worker-<id>/`
- When assigned job:
  1. Receive job type ("wordcount") and file
  2. Process file in 100-byte chunks (handles arbitrarily large files)
  3. Write results to `results.txt`
  4. Send results file back to server
- Supports: wordcount, charcount, capitalize (all uppercase), echo (copy file)

---

## The Journey

### Day 46 (March 2): From Text to Files

**The goal:** Make the task queue handle real files, not just text strings.

**Morning:** Copied week1 code to week2/ directory. Fresh start for the file-based rewrite.

**First problem:** Job struct needs to track files.

Added `file_path` field to the Job struct. Jobs now reference files instead of carrying inline text:
```c
struct Job {
    int job_id;
    int job_type;
    char file_path[MAXFILEPATH];  // New: stores location of job file
    unsigned char results[MAXRESULTSIZE];
    // ...
};
```

**Second problem:** Client needs to send files, not just metadata.

Changed the protocol:
- Old: `./client submit "wordcount hello world"`
- New: `./client submit wordcount /path/to/file.txt`

Client now opens the file, reads the content, and sends it to the server. Added `get_file_size()` function using `fseek()`/`ftell()` trick to determine how much data to send.

**Third problem:** Server needs to receive and store files.

Created `handle_file_transfer()` function that:
1. Creates epoll instance to monitor the client socket
2. Reads incoming data chunks
3. Writes to `./server_storage/job-<id>.txt`

**Afternoon:** Got basic file transfer working, but it's messy.

The server reads chunks into a buffer, then writes to a file. But there's no way to know when the file is complete. TCP streams don't have message boundaries - data just keeps arriving. Could read 10 bytes, then 100, then 50. No pattern.

Also had issues with the command parsing. Client was sending:
1. Header (APPID + JOBSUBMITID)
2. Job type ("wordcount")
3. File content

But server's `recv()` was buffering multiple pieces together. First call got header + job type, second call had nothing left. Need better packet handling.

**By end of day:** File transfer skeleton in place. Client can send files, server can receive them. But it's fragile and has no end-of-file detection. Need a proper protocol.

---

### Day 47 (March 3): File Transfer Protocol Design

**Problem 1:** How do you know when a file transfer is complete over TCP?

TCP is a stream, not packets. If you send 100 bytes, the receiver's `recv()` might get 50 bytes, then 30, then 20. Or all 100 at once. Or 200 bytes if there was data queued. Can't rely on message boundaries.

**Solution:** Sentinel value.

Client sends file in chunks, then sends `"FILE OK"` (8 bytes) as the last thing. Receiver knows transfer is done when it sees that string. Simple, works.

```c
// Client side:
while (bytes_read = fread(buffer, 1, MAXBUFSIZE, file)) {
    send(sockfd, buffer, bytes_read, 0);
}
send(sockfd, "FILE OK", 8, 0);  // Sentinel

// Worker/Server side:
if (strncmp(buf + (bytes_read - 8), "FILE OK", 8) == 0) {
    buf[bytes_read - 8] = '\0';  // Strip sentinel
    fwrite(buf, 1, bytes_read - 8, file);
    return;  // Done
}
```

**Problem 2:** Worker needs to know what to do with the file.

Can't just send "wordcount file.txt" as metadata anymore. Now metadata and file are separate. Server needs to send:
1. Job type ("wordcount")
2. File content

**Solution:** Three-packet protocol.

Client to server:
1. Header: `[APPID][JOBSUBMITID]` (4 bytes)
2. Job spec: `[2-byte length]["wordcount"]`
3. File: `[chunks]["FILE OK"]`

Server to worker:
1. Job packet: `[APPID][WPACKET_NEWJOB][job_id][2-byte spec length][spec]`
2. File: `[chunks]["FILE OK"]`

Worker to server:
1. Status: `[APPID][WPACKET_STATUS][W_SUCCESS]`
2. Results file: `[chunks]["FILE OK"]`

**Problem 3:** TCP stream behavior struck again.

Sent header (4 bytes), then job spec (21 bytes). Server's first `recv()` got all 25 bytes at once. Second `recv()` found nothing, job spec was already consumed.

**Solution:** Read exactly what you expect. 

Changed `recv(sockfd, buf, MAXBUFSIZE, 0)` to `recv(sockfd, buf, 4, 0)` when reading header. Leave the rest on the socket for subsequent reads.

**Afternoon:** Extracted file transfer code into utilities.

Created `utils/file_transfer.c` with:
- `send_file(int sockfd, char *filename)` - Send file with "FILE OK" marker
- `receive_file(char *fname, int sockfd)` - Receive file until "FILE OK"
- `get_file_size(FILE *file)` - fseek/ftell trick for file size

Created `utils/epoll_helper.c` with:
- `create_epoll()` - Make epoll instance
- `add_epoll_fd(int epoll_fd, int new_fd)` - Register FD for monitoring

Now client, server, and worker all use the same file transfer functions. DRY principle.

**By end of day:** File transfers working. Client uploads, server stores, worker receives. Ugly, but functional.

---

### Day 48 (March 4): File-Based Job Processing

**The realization:** Job processing functions still expect text buffers. Need to rewrite everything to work with files.

**Old approach (Week 9):**
```c
int job_wordcount(unsigned char result[MAXRESULTSIZE], 
                  unsigned char content[MAXBUFSIZE]) {
    int count = 0;
    for (int i = 0; i < strlen(content); i++) {
        if (content[i] == ' ') seen_word = 0;
        // ...
    }
    sprintf(result, "word count: %d", count);
}
```

Problems:
- Limited to MAXBUFSIZE (4096 bytes)
- Entire file must fit in memory
- Results limited to MAXRESULTSIZE (2048 bytes)

**New approach (Week 10):**
```c
int job_wordcount(FILE *results, FILE *content) {
    char chunk[MAXFILEREAD];  // 100 bytes
    int wordcount = 0;
    int seen_space = 1;
    
    while ((bytes_read = fread(chunk, 1, MAXFILEREAD, content)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (chunk[i] == ' ') 
                seen_space = 1;
            else if (seen_space) {
                wordcount++;
                seen_space = 0;
            }
        }
    }
    fprintf(results, "%d total words", wordcount);
}
```

Benefits:
- Handles files of any size
- Reads 100 bytes at a time (memory efficient)
- State preserved across chunks (`seen_space` variable)
- Results written directly to file

Rewrote all four job types (wordcount, charcount, echo, capitalize) this way.

**Worker storage design:**

Each worker gets a directory: `./worker_storage/worker-<id>/`

When assigned a job:
1. Receive file → save as `content.txt`
2. Process → write to `results.txt`
3. Send results file back

Added `reset_storage()` function that deletes all files on shutdown. Clean slate for next run.

**Problem:** Worker processes had stale files from previous jobs.

**Solution:** Truncate before writing.
```c
fclose(fopen(fname, "w"));  // Open in write mode (truncates), close immediately
// Later opens in append mode for receiving chunks
```

Hacky but works. Ensures file is empty before receiving new content.

**By end of day:** All job types process files. Can handle multi-megabyte files (tested with 10MB text file). Results come back as files.

---

### Day 49 (March 5): Polish & Monitoring

**Morning:** System worked, but couldn't see what was happening.

Added `print_queue()` function:
```
=== JOBS IN QUEUE ===
1: Job 42
2: Job 43
3: Job 44

```

Server command: type `queue` to see what's waiting.

**Bug found:** Stats weren't updating.

Jobs were completing, but `stats` command showed 0 processed. Found the issue in `manage_worker()`:

```c
// Was missing these:
server->stats->jobs_succeeded++;
server->stats->jobs_processed++;
```

Stats working now. Can watch jobs flow through the system in real-time.

**Cleanup:** Commented out debug prints.

Client was printing "sending job id...", "valid file path", "X bytes sent" for every packet. Noisy. Commented them out. Production-ready output now.

Worker was printing dots (`.....`) while waiting for jobs. Removed that too.

**Storage cleanup:** Added automatic cleanup.

Server deletes all files in `server_storage/` on startup and shutdown. Workers delete their directories on shutdown. Fresh state every run. Important for testing.

---

## How It Works

### File Transfer Protocol

**Chunked reading:**
1. Open file, read MAXBUFSIZE (4096 bytes)
2. Send chunk over socket
3. Clear buffer, repeat
4. Send "FILE OK" when done

**Receiving:**
1. Create epoll instance, wait for socket to be readable
2. Read chunk, write to file
3. Check last 8 bytes for "FILE OK"
4. If found, strip it, write final chunk, return
5. Otherwise, repeat

**Why epoll for receiving?**

Blocking `recv()` would hang if sender is slow. Epoll lets us timeout (50ms) and check if we should abort. Also prep work for handling multiple concurrent transfers (Week 11).

### Job Flow

**Submit job:**
```
Client                 Server                Worker
  |                      |                      |
  |--[header]---------->|                      |
  |--[job spec]-------->|                      |
  |--[file chunks]----->|                      |
  |                    save                    |
  |                   to disk                  |
  |<--[job ID]----------|                      |
  |                      |                      |
  |                      |<--[connected]--------|
  |                      |--[job packet]------->|
  |                      |--[file chunks]------>|
  |                      |                    save
  |                      |                  to disk
  |                      |                  process
  |                      |                   file
  |                      |<--[status]-----------|
  |                      |<--[results file]-----|
  |                    save                    |
  |                   to disk                  |
```

**Retrieve results:**
```
Client                 Server
  |                      |
  |--[result request]--->|
  |--[job ID]---------->|
  |                    check
  |                    status
  |<--[2-byte flag]-----|
  |<--[file chunks]-----|
save to
client_storage/
```

### Worker Processing

Worker receives:
- Job spec: "wordcount"
- File: saved as `./worker_storage/worker-<id>/content.txt`

Process flow:
1. Open `content.txt` for reading
2. Open `results.txt` for writing
3. Read content in 100-byte chunks
4. Process each chunk (count words, capitalize, etc.)
5. Write results
6. Close files
7. Send `results.txt` back to server

**Why 100-byte chunks?**

Small enough to not bloat memory. Large enough to be efficient. Arbitrary choice. Could be 1024 or 512, doesn't matter much.

### Storage Organization

```
project/
├── client_storage/
│   └── results.txt          # Downloaded results
├── server_storage/
│   ├── job-0.txt            # Received from client, sent to worker
│   ├── job-1.txt            # After completion, these become result files
│   └── job-2.txt
├── worker_storage/
│   ├── worker-1234/
│   │   ├── content.txt      # Job file
│   │   └── results.txt      # Processed results
│   └── worker-5678/
│       ├── content.txt
│       └── results.txt
```

Each component has isolated storage. Workers never see each other's files. Clean separation.

---

## Build & Run

### Compile

From the `month3/week2/` directory:

**Server:**
```bash
gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c ./utils/file_transfer.c ./utils/epoll_helper.c -o server
```

**Worker:**
```bash
gcc worker.c ./utils/buffer_manipulation.c ./utils/job_processing.c ./utils/file_transfer.c ./utils/epoll_helper.c -o worker
```

**Client:**
```bash
gcc client.c ./utils/buffer_manipulation.c ./utils/file_transfer.c ./utils/epoll_helper.c -o client
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
Commands while running:
- `stats` - Show jobs processed, success rate, active workers, queue size
- `queue` - List jobs currently in queue
- `quit` - Shutdown gracefully

**Terminal 2+ (Workers):**
```bash
./worker
# Or spawn multiple:
./create_workers 5
```

Each worker gets an ID and waits for jobs.

**Terminal N (Submit Job):**
```bash
# Create a test file:
echo "hello world this is a test file" > ./client_storage/job.txt

# Submit for word counting:
./client submit wordcount "./client_storage/job.txt"
# Returns: Job submitted. ID: 0

# Check status:
./client status 0
# Returns: Job in progress. Worker: 1234
#       or: Job complete. see 'results' for more info.

# Get results:
./client results 0
# Returns: see `./client_storage/results.txt` for results

cat ./client_storage/results.txt
# Output: 7 total words
```

**Job types:**
- `wordcount` - Count words (space-separated sequences)
- `charcount` - Count non-space characters
- `capitalize` - Convert all lowercase to uppercase
- `echo` - Copy file unchanged

---

## What I Learned

### TCP Doesn't Have Message Boundaries

Biggest surprise. I thought sending 100 bytes meant receiver gets 100 bytes in one read. Nope.

TCP is a stream. Could get 50 bytes, then 30, then 20. Or 500 bytes if multiple sends are queued. Operating system decides how to chunk it.

Lesson: Never assume `recv()` returns what you expect. Either:
1. Send length prefix (read 4 bytes for length, then read that many bytes)
2. Use sentinel markers ("FILE OK" in my case)
3. Use fixed-size messages

I used option 2. Works fine for files. Might use option 1 for variable-length metadata in Week 11.

### File I/O Is Simpler Than Buffer Management

Week 9: Constantly juggling buffers, worrying about null termination, checking strlen() vs actual bytes, copying between buffers.

Week 10: Open file, read chunk, process chunk, write chunk. No strlen(). No null terminators. Just bytes.

Files are cleaner. Especially for large data.

### Streaming Algorithms Require State Management

Wordcount example: What if a word spans two chunks?

```
Chunk 1: "hello wor"
Chunk 2: "ld goodbye"
```

Can't count "wor" and "ld" as separate words. Need to track: "Did the previous chunk end mid-word?"

Solution: `seen_space` flag. Preserved across chunks. If last character wasn't a space and first character of next chunk isn't a space, they're part of the same word.

This is why MapReduce and streaming systems are hard. State management across chunks/nodes.

### Epoll Is Overkill For Single File Transfers

I'm using epoll (create instance, register FD, wait for events) just to read one file from one socket. Could've used blocking `recv()` with a timeout via `setsockopt(SO_RCVTIMEO)`.

But Week 11 might have workers handling multiple jobs concurrently. Server might receive multiple files at once. Epoll scales. Blocking calls don't.

Over-engineering now, but it's practice. Real systems use epoll/kqueue for everything.

### Debug Prints Are Crucial Until They're Not

During development: print everything. Every recv(), every send(), every state change.

In production: silent unless there's an error.

I left most debug prints in during Days 46-47. Removed them Day 48 once everything worked. Should've been more disciplined—commenting as I go instead of batch cleanup.

---

## Known Issues

### Binary Protocol Makes Debugging Hard

Can't just `tcpdump` and see what's happening. Packets are binary (2-byte integers, no delimiters). Would need a custom parser.

JSON would be human-readable. But binary is faster and smaller. Trade-off.

### No File Type Validation

Server accepts any file. Worker processes it. What if someone uploads a 10GB video? Worker reads it 100 bytes at a time, but that's still going to take forever.

Should add:
- File size limits (reject > 100MB)
- File type whitelist (only .txt, .jpg, .png, etc.)
- Timeout on job execution

### Workers Block On File I/O

`fread()` blocks until data is available. If disk is slow (network mount, old HDD), worker hangs.

Should use async I/O or thread pool for file operations. Complicated. Week 11 problem.

### No Partial Results

If a job processes 90% of a file then crashes, all progress is lost. Have to start over.

Real systems (Hadoop, Spark) checkpoint progress. Can resume from last chunk. Not implemented here.

### File Sentinel Can Appear In Data

What if someone's file contains "FILE OK"? My code would think transfer is done, truncate the file.

Should use length-prefixed protocol instead:
```
[4-byte file size][file data]
```

Read exactly `file size` bytes, then stop. No ambiguity.

But that requires knowing file size upfront. My current approach reads file in chunks without loading into memory. Trade-offs.

### No Encryption

Files are sent in plaintext. Anyone sniffing the network can read everything.

Production systems use TLS. Adding that here would obscure the learning. But it's a real problem.

---

## Files & Structure

### Main Programs
- `server.c` (697 lines) - Dual-port epoll server managing clients and workers
- `worker.c` (302 lines) - File processor with isolated storage per worker
- `client.c` (227 lines) - CLI for job submission and result retrieval
- `submit_jobs.c` (127 lines) - Load testing utility (not updated for files yet)
- `create_workers.c` (63 lines) - Spawn multiple worker processes

### Utilities (NEW in Week 10)
- `utils/file_transfer.c` (90 lines) - Generic file send/receive with "FILE OK" protocol
- `utils/epoll_helper.c` (28 lines) - Reusable epoll instance creation and FD registration

### Utilities (From Week 9)
- `utils/jobs.c/h` - Job struct management (added `job_spec` field)
- `utils/workers.c/h` - Worker struct management
- `utils/job_queue.c/h` - FIFO queue (added `print_queue()` function)
- `utils/job_processing.c/h` - Job execution (completely rewritten for FILE pointers)
- `utils/buffer_manipulation.c/h` - Binary packing/unpacking
- `utils/time_custom.c/h` - Timing utilities

### Configuration
- `common.h` - Shared constants (ports, buffer sizes, packet IDs, added `MAXFILEREAD`)

**Total: ~1,400 lines of code + ~200 lines of new utilities = ~1,600 lines**

---

## What's Next

**Week 11 (Planned): Real File Processing**

The infrastructure works. Now make it useful:

- **Image processing:** Resize images (use stb_image library)
- **File compression:** tar/gzip files
- **Document conversion:** Markdown to HTML
- **Threading:** Workers handle multiple jobs concurrently (pthreads)

**Later:**
- Job persistence (SQLite) - survive server crashes
- Distributed workers across machines (not just localhost)
- Better monitoring (web dashboard)
- Priority queues (urgent jobs jump the line)

But first: make workers do something actually valuable. Image thumbnails feel like the right next step.

---

## Reflections

This week felt like real infrastructure work. Week 9 was networking + data structures. Week 10 is about I/O, file systems, streaming algorithms.

The hard part wasn't the code—it was figuring out the protocols. How do you reliably transfer a file over TCP? How do you process arbitrarily large files with fixed memory? How do you keep worker storage isolated?

These aren't algorithm problems. They're system design problems. No right answer, just trade-offs. I picked simple solutions (sentinel markers, 100-byte chunks, directory per worker) that are easy to understand and debug.

Production systems would do this differently (length-prefixed, async I/O, thread pools). But the principles are the same: stream data, maintain state, isolate failures.

Building it from scratch makes you appreciate what frameworks hide. Every file upload API, every background job system (Sidekiq, Celery, Bull)—they all solve these problems. Now I know what problems those are.