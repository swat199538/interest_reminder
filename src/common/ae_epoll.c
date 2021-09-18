//
// Created by swat on 9/8/2021.
//
#include "sys/epoll.h"

typedef struct aeApiState{
    int epfd;
    struct epoll_event events[AE_SETSIZE];
} aeApiState;

static char *aeApiName(){
    return "epoll";
}

static int aeApiCreate(aeEventLoop *eventLoop){

    aeApiState *state = (aeApiState*)zmalloc(sizeof(aeApiState));

    if(!state) return -1;
    state->epfd = epoll_create(1024);
    if (state->epfd == -1) return -1;
    eventLoop->apiData = state;
    return 0;
}

