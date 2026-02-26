#ifndef COMMON_H
#define COMMON_H

// max values
#define MAXRESULTSIZE 100
#define MAXJOBCOMMANDSIZE 100
#define MAXJOBMETADATASIZE 50
#define MAXBACKLOG 10
#define MAXEPOLLEVENTS 100
#define MAXBUFSIZE 512

// worker status
#define W_READY 0
#define W_SUCCESS 1
#define W_FAILURE -1
#define W_BUSY 2

// job status
#define J_IN_QUEUE 0
#define J_SUCCESS 1
#define J_FAILURE 3
#define J_IN_PROGRESS 2

// server config
#define CLIENT_PORT "1209"
#define WORKER_PORT "1205"

// ids
#define APPID 4379
#define JOBSUBMITID 808
#define JOBSTATUSID 909
#define JOBRESULTID 707

// worker packet types
#define WPACKET_CONNECTED 901
#define WPACKET_NEWJOB 902
#define WPACKET_STATUS 903
#define WPACKET_CANCELJOB 904

// job types
#define JTYPE_WORDCOUNT 2500
#define JTYPE_ECHO 2501
#define JTYPE_CAPITALIZE 2502

/*

Job creation format:

./client submit "[JOB_TYPE] [MODS] [JOB-SPECIFIC META-DATA]" -> "Job Submitted with ID: [JOB_ID]"

./client status [JOB_ID] -> "Job -[JOB_ID]- Status - [STATUS]"

./client results [JOB_ID] -> [RESULTS]                                                  // Given a job has string/text output
                             "Job output found in `output/job-[JOB_ID]"                 // Given a job has file/non-text output
                             "Job [JOB_ID] incomplete - use `./client status [JOB_ID]"  // Given a job is not yet completed
                             "Job [JOB_ID] not found"                                   // Given a job does not exist




*/


#endif