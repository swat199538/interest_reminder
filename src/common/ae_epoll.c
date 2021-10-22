//
// Created by swat on 9/8/2021.
//
#include <unistd.h>
#include "sys/epoll.h"
#include "zmalloc.h"
#include <sys/time.h>

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

static void aeApiFree(aeEventLoop *eventLoop){
    aeApiState *state = eventLoop->apiData;
    close(state->epfd);
    zfree(state);
}

static int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask){
    aeApiState *state = eventLoop->apiData;

    struct epoll_event ee;
    int op = eventLoop->events[fd].mask == AE_NONE ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;

    mask |= eventLoop->events[fd].mask;

    if (mask & AE_READABLE) ee.events |= EPOLLIN; //epoll can read
    if (mask & AE_WRITEABLE) ee.events |= EPOLLOUT; //epoll can write

    ee.data.u64 = 0;
    ee.data.fd = fd;

    if (epoll_ctl(state->epfd, op, fd, &ee) == -1) return -1;
    return 0;
}

static void aeApiDelEvent(aeEventLoop *eventLoop, int fd, int delmask){
    aeApiState *state = eventLoop->apiData;
    struct epoll_event ee;
    int mask = eventLoop->events[fd].mask & (~delmask);

    ee.events = 0;
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITEABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (mask != AE_NONE){
        epoll_ctl(state->epfd, EPOLL_CTL_MOD, fd, &ee);
    } else{
        epoll_ctl(state->epfd, EPOLL_CTL_DEL, fd, &ee);
    }
}

static int aeApiPool(aeEventLoop *eventLoop, struct timeval *tvp){
    aeApiState *state = eventLoop->apiData;
    int retval, numevents = 0;

    retval = epoll_wait(state->epfd, state->events, AE_SETSIZE,
               tvp ?  (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);

    if (retval > 0){
        int j;

        numevents = retval;
        for (j = 0; j < numevents ; j++) {
            int mask = 0;
            struct epoll_event *e = state->events+j;
            if (e->events & EPOLLIN)    mask |= AE_READABLE;
            if (e->events & EPOLLOUT)   mask |= AE_WRITEABLE;
            eventLoop->fired[j].fd = e->data.fd;
            eventLoop->fired[j].mask = mask;
        }
    }

    return numevents;
}