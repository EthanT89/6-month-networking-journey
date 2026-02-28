server: `gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c -o server`
client: `gcc client.c ./utils/buffer_manipulation.c -o client`
worker: `gcc worker.c ./utils/buffer_manipulation.c -o worker`
submitjobs: ` gcc submit_jobs.c ./utils/buffer_manipulation.c -o submit_jobs`
createworkers: `gcc create_workers.c -o create_workers`