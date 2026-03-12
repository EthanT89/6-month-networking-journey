#ifndef COMMON_H
#define COMMON_H

// max values
#define MAXRESULTSIZE 2048
#define MAXJOBCOMMANDSIZE 2048
#define MAXJOBMETADATASIZE 2048
#define MAXBACKLOG 10
#define MAXEPOLLEVENTS 100
#define MAXBUFSIZE  4096
#define MAXFILEPATH 100
#define MAXFILEREAD 100  // chunk size for streaming file reads
#define MAXFILEEXT 5

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
#define JOBID 606

// file types
#define IMG_FILE 755
#define TXT_FILE 756

// server response types
#define SERVER_MSG 9090
#define SERVER_FILE_TRANSFER 9091

// worker packet types
#define WPACKET_CONNECTED 901
#define WPACKET_NEWJOB 902
#define WPACKET_STATUS 903
#define WPACKET_CANCELJOB 904
#define WPACKET_RESULTS 905

// job types
#define JTYPE_WORDCOUNT 2500
#define JTYPE_ECHO 2501
#define JTYPE_CAPITALIZE 2502
#define JTYPE_CHARCOUNT 2503
#define JTYPE_CSVSTATS 2504
#define JTYPE_CSVSORT 2505
#define JTYPE_CSVFILTER 2506
// img jobs
#define JTYPE_SCALE 2550
#define JTYPE_RESIZE 2551
#define JTYPE_FILTER 2552
#define JTYPE_FLIPX 2553
#define JTYPE_FLIPY 2554
#define JTYPE_ROTATE 2555
#define JTYPE_CHARCOAL 2556
#define JTYPE_MONOCHROME 2557
#define JTYPE_STENCIL 2558

// worker error codes
#define WERR_NONE 1
#define WERR_UNKNOWN -1
#define WERR_INVALIDJOB -2
#define WERR_WORKERQUIT -3

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