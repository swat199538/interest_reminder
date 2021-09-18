//
// Created by swat on 9/7/2021.
//

#include "ae.h"
#include "config.h"

#ifdef HAVE_EPOLL
#include "ae_epoll.c"
#endif

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

static void aeAddMillisecondsToNow(long long milliseconds, long *sec, long *ms){
    long cur_sec,cur_ms, when_sec, when_ms;

    aeGetTime(&cur_sec, &cur_ms);

    when_sec = cur_sec + milliseconds /1000;
    when_ms = cur_ms + milliseconds%1000;

    if (when_ms >= 1000) {
        when_sec ++;
        when_ms -= 1000;
    }

    *sec = when_sec;
    *ms = when_ms;
}

static int processTimeEvents(aeEventLoop *eventLoop){
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;

    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId - 1;

    while (te){
        long now_sec,now_ms;
        long long id;

        if (te->id > maxId){
            te = te->next;
            continue;
        }
        aeGetTime(&now_sec, &now_ms);
        if (now_sec > te->when_sec ||
            (now_sec == te->when_sec && now_ms >= te->when_ms)) {
            int retval;
            id = te->id;
            retval = te->timeProc(eventLoop, id, te->clientData);
            processed++;

            if (retval != AE_NOMORE){
                aeAddMillisecondsToNow(retval, &te->when_sec, &te->when_ms);
            } else{
                aeDeleteTimeEvent(eventLoop, id);
            }

        } else{
            te = te->next;
        }
    }

    return processed;
}

aeEventLoop *aeCreateEventLoop(){
    aeEventLoop *eventLoop;
    int i;

    eventLoop = zmalloc(sizeof(*eventLoop));
    if (!eventLoop) eventLoop = NULL;
    eventLoop->timeEventHead = NULL;
    eventLoop->timeEventNextId = 0;
    eventLoop->stop=0;
    eventLoop->maxFd=-1;
    eventLoop->beforeSleep = NULL;
    if (aeApiCreate(eventLoop) == -1){
        zfree(eventLoop);
        return NULL;
    }

    for (i = 0; i < AE_SETSIZE ; i++)
        eventLoop->events[i].mask = AE_NONE;
    return eventLoop;
}

char *aeGetApiName(void) {
    return aeApiName();
}

void aeDeleteEventLoop(aeEventLoop *eventLoop){
    zfree(eventLoop);
}

void aeStop(aeEventLoop *eventLoop){
    eventLoop->stop = 1;
}

int aeProcessEvents(aeEventLoop *eventLoop, int flags){
    int processed = 0;

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
        processed += processTimeEvents(eventLoop);

    return processed;
}

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
      aeFileProc *proc, void *clientData)
{
    if (fd >= AE_SETSIZE) return AE_ERR;

    aeFileEvent *fe = &eventLoop->events[fd];

    if ()

    return AE_OK;
}

long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
     aeTimeProc *proc, void *clientData,
     aeEventFinalizerProc *finalizerProc)
{
    long long id = eventLoop->timeEventNextId++;
    aeTimeEvent *te;

    te = zmalloc(sizeof(*te));
    if (te==NULL) return AE_ERR;

    te->id = id;
    aeAddMillisecondsToNow(milliseconds, &te->when_sec, &te->when_ms);
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    te->clientData = clientData;
    te->next = eventLoop->timeEventHead;
    eventLoop->timeEventHead = te;
    return id;
}

int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id){
    aeTimeEvent *te, *prev = NULL;

    te = eventLoop->timeEventHead;

    while (te){
        if (te->id == id){
            if (prev == NULL)
                eventLoop->timeEventHead = te->next;
            else
                prev->next = te->next;
            if (te->finalizerProc)
                te->finalizerProc(eventLoop, te->clientData);

            zfree(te);
            return AE_OK;
        }
        prev = te;
        te = te->next;
    }

    return AE_ERR;
}

void aeMain(aeEventLoop *eventLoop){
    eventLoop->stop = 0;
    while (!eventLoop->stop){
        if (eventLoop->beforeSleep)
            eventLoop->beforeSleep(eventLoop);
        aeProcessEvents(eventLoop, AE_ALL_EVENTS);
    }
}

void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforeSleep){
    eventLoop->beforeSleep = beforeSleep;
}