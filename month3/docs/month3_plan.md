# Month 3: Systems Programming Fundamentals
*Days 41-60 | February 24 - March 13, 2026*

## Overview
Building a distributed task queue system to learn core systems programming concepts: multithreading, network protocols, state management, and scaling. This project teaches the fundamental skills used in backend infrastructure at companies like Roblox.

## Project: Distributed Task Queue System
**Core Challenge:** Build a system where clients can submit jobs, workers process them concurrently, and the system scales across multiple machines.

**Why this project:**
- Forces learning of essential systems concepts (threading, protocols, scaling)
- Directly applicable to infrastructure roles at tech companies
- Concrete and useful - can actually use for automating tasks
- Natural progression from basic networking to distributed systems

## Technical Learning Goals

### Core Systems Concepts
- **Multithreading** - concurrent job processing, thread-safe data structures
- **Async I/O** - handling many client connections efficiently
- **Protocol design** - efficient job submission and status protocols
- **State management** - job persistence, failure recovery, status tracking
- **Process management** - worker coordination, load distribution

### Infrastructure Patterns
- **Producer-consumer queues** - fundamental backend pattern
- **Work distribution** - load balancing across workers
- **Fault tolerance** - handling worker failures and job recovery
- **Monitoring and observability** - tracking system performance
- **Deployment** - running distributed services in production

## Week Structure

### Week 9 (Days 41-45): Basic Task Queue
**Goal:** Build single-threaded queue server with simple job processing
**Daily sessions:** 2-4:30pm weekdays (2.5 hours each)
- Design job submission protocol and data structures
- Implement basic queue server that accepts jobs
- Create simple worker that processes text-based jobs
- Add job status tracking and result retrieval

### Week 10 (Days 46-50): Multi-threaded Processing
**Goal:** Add concurrent job processing with thread safety
**Daily sessions:** 2-4:30pm weekdays (2.5 hours each)
- Learn pthread basics and thread synchronization
- Implement thread-safe job queue with producer-consumer pattern
- Add multiple worker threads for concurrent processing
- Implement more complex job types (file processing, calculations)

### Week 11 (Days 51-55): Distribution and Persistence
**Goal:** Scale to multiple worker machines with job persistence
**Daily sessions:** 2-4:30pm weekdays (2.5 hours each)
- Implement job persistence to disk for crash recovery
- Add distributed workers on separate machines
- Create job priority system and retry logic
- Build basic monitoring and statistics collection

### Week 12 (Days 56-60): Production Deployment
**Goal:** Deploy system to cloud with production features
**Daily sessions:** 2-4:30pm weekdays (2.5 hours each)
- Deploy queue server and workers to cloud instances
- Add advanced features (scheduled jobs, job dependencies)
- Implement comprehensive monitoring and alerting
- Load testing and performance optimization

## Success Metrics

### Technical Achievements
- [ ] Handle 1000+ concurrent job submissions
- [ ] Process multiple job types with different resource requirements
- [ ] Survive server crashes without losing jobs
- [ ] Scale across multiple worker machines
- [ ] Deploy and run stably in cloud environment

### Learning Outcomes
- [ ] Comfortable with multithreading and synchronization primitives
- [ ] Understanding of producer-consumer and work queue patterns
- [ ] Experience with network protocol design for backend services
- [ ] Knowledge of job persistence and failure recovery strategies
- [ ] Skills in distributed system deployment and monitoring

## Career Positioning
**Primary framing:** "Backend Systems Engineer"
**Skills demonstrate:** Multithreading, distributed systems, protocol design, service deployment
**Target roles:** Infrastructure teams at tech companies (Roblox, Google, Meta), backend roles at startups

## Example Job Types to Implement

### Week 9: Simple Jobs
- Text processing (word count, format conversion)
- Basic calculations (prime numbers, factorial)
- File operations (compression, format conversion)
- Simple HTTP requests (web scraping basics)

### Week 10: Complex Jobs
- Image processing (resize, format conversion, filters)
- Data analysis (CSV processing, statistical calculations)
- Archive operations (tar/zip creation and extraction)
- Email sending (SMTP integration)

### Week 11: Advanced Jobs
- Report generation from data files
- Batch file processing with multiple steps
- Web scraping with multiple pages
- Log analysis and metric extraction

### Week 12: Production Jobs
- Scheduled maintenance tasks
- Backup and sync operations
- Performance monitoring jobs
- Health check and alerting jobs

## Architecture Overview

```
[Clients] → [Queue Server] → [Worker 1]
                          → [Worker 2]
                          → [Worker N]
```

**Components:**
- **Queue Server:** Accepts jobs, manages queue, tracks status
- **Workers:** Process jobs, report results back to server
- **Clients:** Submit jobs, check status, retrieve results
- **Storage:** Persistent job storage for crash recovery

## Roblox Relevance
This project directly prepares for:
- **Engine Team:** Understanding concurrent systems, job processing for asset pipelines
- **Infrastructure Team:** Backend service patterns, distributed system management
- **Foundation AI Team:** Job queues for ML model training and inference

Task queues are fundamental to all major backend systems - this knowledge applies everywhere.

## Reflection Points
- **Week 9:** How do job queues compare to the networking patterns I've learned?
- **Week 10:** What are the key challenges in thread-safe programming?
- **Week 11:** How do distributed systems handle partial failures?
- **Week 12:** What would it take to scale this to handle millions of jobs per day?

## Next Steps After Month 3
With strong systems fundamentals:
- **Option A:** Advanced distributed systems (consensus algorithms, replication)
- **Option B:** Performance engineering (profiling, optimization, assembly)
- **Option C:** Database systems (storage engines, query processing)

The systems programming foundation from Month 3 opens doors to infrastructure roles at any tech company.

---

*This project focuses on practical systems knowledge that directly applies to backend infrastructure roles. The task queue is just the vehicle - the real value is in multithreading, protocols, and distributed systems concepts.*