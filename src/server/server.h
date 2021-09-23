//
// Created by wangl on 8/26/2021.
//

#ifndef IREMINDER_SERVER_H
#define IREMINDER_SERVER_H

#include <bits/types/sig_atomic_t.h>
#include "../common/ae.h"
#include "../common/anet.h"

#define IR_SERVER_PORT 9538
#define IR_SERVER_BIND_ADDR "127.0.0.0"

/* Log levels */
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_RAW (1<<10) /* Modifier to log without timestamp */


extern struct iRServer server;

struct iRServer{
    int pid; // process id
    int thread_id; //main thread id
    char *bindaddr;//bind ip address
    int port; // tcp listen port
    char *pid_file; // pid file path
    volatile sig_atomic_t shutdown_asap;
    volatile sig_atomic_t loading; /* We are loading data from disk if true */
    char *logfile;
    int verbosity;
    int daemonize;  /* True if running as a daemon */
    aeEventLoop *el; //event loop
    int fd;//socket fd
    char neterr[ANET_ERR_LEN];
    long long stat_numconnections;
};


void initServerConfig(void); // init server config

void initServer(void); // init server

void setupSignalHandlers(void);

void serverLogFromHandler(int level, const char *msg);

#endif //IREMINDER_SERVER_H
