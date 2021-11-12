//
// Created by wangl on 8/26/2021.
//

#ifndef IREMINDER_SERVER_H
#define IREMINDER_SERVER_H

#include <bits/types/sig_atomic_t.h>
#include <stddef.h>
#include <bits/types/time_t.h>
#include "../common/ae.h"
#include "../common/anet.h"
#include "../common/adlist.h"
#include "../common/sds.h"

#define IR_SERVER_PORT 9538
#define IR_SERVER_BIND_ADDR "127.0.0.1"
#define IR_IOBUF_LEN 1024
#define IR_PROJECT_LEN 1024

/* Log levels */
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_RAW (1<<10) /* Modifier to log without timestamp */

typedef struct interestObject{
    sds tag;
    sds name;
    sds bank;
    time_t depositDate;
    time_t expirationDate;
    size_t amount;
    float rate;// interest rate
    unsigned int payoutDay;//Dividend payout time
    float grossInterest;
    float interestPaid;
    char finished;
} inObj;

extern struct iRServer server;

struct iRServer{
    int pid; // process id
    list *client;//connected client
    unsigned long int thread_id; //main thread id
    char *bindaddr;//bind ip address
    int port; // tcp listen port
    char *pid_file; // pid file path
    volatile sig_atomic_t shutdown_asap;
    volatile sig_atomic_t loading; /* We are loading value from disk if true */
    char *logfile;
    int verbosity;
    int daemonize;  /* True if running as a daemon */
    aeEventLoop *el; //event loop
    list *objfreelist;/* A list of freed objects to avoid malloc() */
    int fd;//socket fd
    char neterr[ANET_ERR_LEN];
    long long stat_numconnections;
    unsigned int maxClients;
    inObj **project;
    unsigned int projectCount;
};

void initServerConfig(void); // init server config
void initServer(void); // init server
void serverLogFromHandler(int level, const char *msg);

#endif //IREMINDER_SERVER_H
