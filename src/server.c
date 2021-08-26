//
// Created by wangl on 8/26/2021.
//

#include "server.h"
#include <unistd.h>
#include "stddef.h"
#include "time.h"
#include "pthread.h"

/*================================= Globals ================================= */
struct iRServer server;


/*============================ Utility functions ============================ */
void initServerConfig(){
    server.port = IR_SERVER_PORT;
    server.bindaddr = IR_SERVER_BIND_ADDR;
}


void initServer(){
    server.pid = getpid();
    server.thread_id = pthread_self();
}


/*================================= Main ================================= */
int main(int argc, char** argv){

    time_t start;

    initServerConfig();

    initServer();

    start = time(NULL)

    return 0;
}
