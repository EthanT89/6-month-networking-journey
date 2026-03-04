

### Compile

From the `month3/week2/` directory:

**Server:**
```bash
gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c ./utils/file_transfer.c -o server
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
