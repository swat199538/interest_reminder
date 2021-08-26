//
// Created by wangl on 8/26/2021.
//

#ifndef IREMINDER_SERVER_H
#define IREMINDER_SERVER_H

#define IR_SERVER_PORT 9538
#define IR_SERVER_BIND_ADDR "127.0.0.0"

extern struct iRServer server;

struct iRServer{
    int pid; // process id
    int thread_id; //main thread id
    char *bindaddr;//bind ip address
    int port; // tcp listen port
    char *pid_file; // pid file path
};


void initServerConfig(); // init server config

void initServer(); // init server

#endif //IREMINDER_SERVER_H
