//
// Created by swat on 9/7/2021.
//

#include "ae.h"
#include "ae_epoll.c"
#include "zmalloc.h"
#include <sys/time.h>

static aeTimeEvent *aeSearchNearsTimer(aeEventLoop *eventLoop){
    aeTimeEvent *te = eventLoop->timeEventHead;
    aeTimeEvent *nearest = NULL;

    while (te){
        if (!nearest || te->when_sec < nearest->when_sec ||
                (te->when_sec == nearest->when_sec &&
                te->when_ms < nearest->when_ms))
            nearest = te;
        te = te->next;
    }

    return nearest;
}

static void aeGetTime(long *seconds, long *milliseconds){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec/1000;
}

static int processTimeEvents(aeEventLoop *eventLoop){
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;

    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId - 1;

    while (te){
        long now_sec,now_ms;

    }

}

aeEventLoop *aeCreateEventLoop(){
    aeEventLoop *eventLoop;
    int i;

    eventLoop = (aeEventLoop *)zmalloc(sizeof(aeEventLoop));
    if (!eventLoop) eventLoop = NULL;
    eventLoop->timeEventHead = NULL;
    eventLoop->timeEventNextId = 0;
    eventLoop->stop=0;
    eventLoop->maxFd=-1;
    eventLoop->beforeSleepProc = NULL;

    return eventLoop;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop){
    zfree(eventLoop);
}

void aeStop(aeEventLoop *eventLoop){
    eventLoop->stop = 1;
}

int aeProcessEvents(aeEventLoop *eventLoop, int flags){
    int processed = 0, numevents;

    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) return 0;

    if (eventLoop->maxFd != -1 ||
        ((flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS))){
        aeTimeEvent *shortest = NULL;
        struct timeval tv, *tvp;
        if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT))
            shortest = aeSearchNearsTimer(eventLoop);
        if (shortest){
            long now_sec, now_ms;
            aeGetTime(&now_sec, &now_ms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec - now_sec;
            if (shortest->when_ms < now_ms){
                tvp->tv_usec = ((shortest->when_ms+1000) - now_ms) * 1000;
                tvp->tv_sec--;
            } else{
                tvp->tv_usec = (shortest->when_ms - now_ms)*1000;
            }
            if (tvp->tv_usec < 0) tv.tv_usec =0;
            if (tvp->tv_sec < 0) tv.tv_sec = 0;
        } else{
            if (flags & AE_DONT_WAIT){
                tv.tv_sec = tv.tv_usec =0;
                tvp = &tv;
            } else{
                tvp = NULL;
            }
        }
    }

    if (flags & AE_TIME_EVENTS)
//        processed +=

    return processed;
}



int aeMain(aeEventLoop *eventLoop){
    eventLoop->stop = 0;
    while (!eventLoop->stop){
        if (eventLoop->beforeSleepProc)
            eventLoop->beforeSleepProc(eventLoop);
        aeProcessEvents(eventLoop, AE_ALL_EVENTS);
    }
}