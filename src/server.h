//
// Created by wangl on 8/26/2021.
//

#ifndef IREMINDER_SERVER_H
#define IREMINDER_SERVER_H

void initServer();

typedef struct iRServer{
    int pid; // process id
    int thread_id; //main thread id
    int port; // tcp listen port
    char *pid_file; // pid file path
} iRServer;

#endif //IREMINDER_SERVER_H
