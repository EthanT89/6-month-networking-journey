# Week 9: Basic Task Queue
*Days 41-45 | February 24-28, 2026*
*Daily Deep Work Sessions: 2-4:30pm (2.5 hours each)*

## This Week's Challenge
Build a working task queue system where clients can submit jobs and a server processes them. This introduces fundamental systems programming patterns: protocol design, job state management, and basic concurrency concepts.

**Core concept to master:** Producer-consumer pattern with network interface

## Learning Objectives
By Friday I should understand:
- How to design a simple network protocol for job submission
- Basic job state management (pending, running, completed, failed)
- File I/O and process execution for job processing
- Foundation for adding threading next week

## Daily Sessions (2-4:30pm)

### Day 41 (Monday): Protocol Design & Basic Server
**2:00-2:45pm - Protocol Planning:**
- Design simple text-based protocol for job submission
- Plan job data structure (ID, command, status, result)
- Sketch basic client-server interaction flow

**2:45-4:00pm - Basic Server Implementation:**
- Create server that accepts TCP connections
- Implement job submission handler
- Basic job storage in memory (array/linked list)

**4:00-4:30pm - Testing:**
- Simple client that can connect and submit jobs
- Test basic job submission and storage

**Deliverable:** Server that accepts job submissions and stores them

### Day 42 (Tuesday): Job Processing Engine
**2:00-3:30pm - Job Execution:**
- Implement job processor that executes shell commands
- Add job status tracking (pending → running → completed)
- Basic result capture (stdout, stderr, exit code)

**3:30-4:15pm - Job Types:**
- Implement different job types:
  - Echo commands ("echo hello world")
  - Simple calculations ("expr 5 + 3")
  - File operations ("ls -la", "wc file.txt")

**4:15-4:30pm - Testing:**
- Submit various job types
- Verify results are captured correctly

**Deliverable:** Server that can execute submitted jobs and track results

### Day 43 (Wednesday): Client Interface & Status Queries
**2:00-3:00pm - Client Commands:**
- Extend client to support multiple operations:
  - Submit job: `./client submit "command"`
  - Check status: `./client status <job-id>`
  - Get result: `./client result <job-id>`

**3:00-4:00pm - Status Protocol:**
- Implement status query handling on server
- Return job progress and results to clients
- Add basic error handling for invalid job IDs

**4:00-4:30pm - User Experience:**
- Make client more user-friendly
- Add command-line argument parsing
- Test complete job lifecycle

**Deliverable:** Complete client-server system for job submission and monitoring

### Day 44 (Thursday): Job Persistence & Recovery
**2:00-3:15pm - Persistence:**
- Save jobs to disk (simple text file format)
- Load jobs on server startup
- Maintain job state across server restarts

**3:15-4:15pm - Error Handling:**
- Handle job failures gracefully
- Add timeout handling for long-running jobs
- Implement basic retry logic for failed jobs

**4:15-4:30pm - Testing:**
- Test server restart with pending jobs
- Submit failing jobs and verify error handling

**Deliverable:** Robust job queue that survives server restarts

### Day 45 (Friday): Testing & Documentation
**2:00-3:00pm - Stress Testing:**
- Submit many jobs quickly
- Test with different job types
- Identify performance bottlenecks

**3:00-4:00pm - More Job Types:**
- File processing: `gzip <file>`, `convert image.jpg thumb.png`
- Math: `factor 12345`, calculate primes
- Network: `ping google.com`, `curl example.com`

**4:00-4:30pm - Week Reflection:**
- Document what worked well
- Identify areas for improvement
- Plan for adding threading in Week 10

**Deliverable:** Stable job queue ready for threading improvements

## Key Concepts This Week

### Job Lifecycle
```
PENDING → RUNNING → COMPLETED
       → RUNNING → FAILED
```

### Basic Protocol Design
```
SUBMIT <command>        → JOB_ID <id>
STATUS <id>            → STATUS <status> <progress>
RESULT <id>            → RESULT <stdout> <stderr> <exit_code>
```

### Job Data Structure
```c
struct Job {
    int id;
    char command[MAX_COMMAND_LEN];
    enum JobStatus status;  // PENDING, RUNNING, COMPLETED, FAILED
    char result[MAX_RESULT_LEN];
    int exit_code;
    time_t submitted_at;
    time_t started_at;
    time_t completed_at;
};
```

## Realistic Scope for 2.5 Hours/Day

### What's Achievable This Week
- Basic job submission and execution
- Simple status tracking and queries
- Job persistence to survive restarts
- Handle 10-20 concurrent job submissions

### What's NOT Achievable This Week
- Multithreading (that's Week 10)
- Distributed workers
- Complex job scheduling
- High performance optimization

## Success Metrics

### Technical
- [ ] Server handles job submission, execution, and status queries
- [ ] Jobs persist across server restarts
- [ ] Can process various job types (shell commands, file operations)
- [ ] Basic error handling for failed jobs

### Learning
- [ ] Understand producer-consumer pattern at conceptual level
- [ ] Comfortable with basic network protocol design
- [ ] Know how to execute shell commands from C program
- [ ] Familiar with job state management patterns

## Week 9 → Week 10 Transition
The single-threaded foundation built this week becomes the basis for adding multithreading. Next week: separate job processing from client handling using threads and thread-safe queues.

## Reading for the Week
- **Unix Network Programming** (Stevens) - Chapter 1 (networking basics review)
- **"Producer-Consumer Problem"** - basic computer science concept
- **System calls for process execution** - `fork()`, `exec()`, `wait()` documentation

---

*Week 9 is about building a solid, working foundation. Keep it simple and focus on the core job queue mechanics.*