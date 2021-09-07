//
// This is a simple event loop imitate redis ae
//

#ifndef INTEREST_REMINDER_AE_H
#define INTEREST_REMINDER_AE_H

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS | AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

struct aeEventLoop;

typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *aeEventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

typedef struct aeTimeEvent{
    long long id;
    long when_sec;
    long when_ms;
    aeTimeProc *timeProc;
    aeEventFinalizerProc *eventFinalizerProc;
    void *clientData;
    struct aeTimeEvent *next;
} aeTimeEvent;

typedef struct aeEventLoop{
    int maxFd;
    long long timeEventNextId;
    aeTimeEvent *timeEventHead;
    int stop;
    aeBeforeSleepProc *beforeSleepProc;
} aeEventLoop;

#endif //INTEREST_REMINDER_AE_H
