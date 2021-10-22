//
// This is a simple event loop imitate redis ae
//

#ifndef INTEREST_REMINDER_AE_H
#define INTEREST_REMINDER_AE_H

#define AE_SETSIZE (1024*10)

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE  1
#define AE_WRITEABLE 2

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1

struct aeEventLoop;

typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *aeEventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

typedef struct aeFileEvent{
    int mask;
    aeFileProc *rFileProc;
    aeFileProc *wFileProc;
    void *clientData;
} aeFileEvent;

typedef struct aeTimeEvent{
    long long id;
    long when_sec;
    long when_ms;
    aeTimeProc *timeProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeTimeEvent *next;
} aeTimeEvent;

typedef struct aeFiredEvent{
    int fd;
    int mask;
} aeFiredEvent;

typedef struct aeEventLoop{
    int maxFd;
    long long timeEventNextId;
    aeFileEvent events[AE_SETSIZE];
    aeFiredEvent fired[AE_SETSIZE];
    aeTimeEvent *timeEventHead;
    int stop;
    void *apiData;
    aeBeforeSleepProc *beforeSleep;
} aeEventLoop;

aeEventLoop *aeCreateEventLoop();
void aeDeleteEventLoop(aeEventLoop *eventLoop);
void aeStop (aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
          aeFileProc *proc, void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
          aeTimeProc *proc, void *clientData,
          aeEventFinalizerProc *finalizerProc);
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);
int aeProcessEvents(aeEventLoop *eventLoop, int flags);
int aeWait(int fd, int mask, long long milliseconds);
void aeMain(aeEventLoop *eventLoop);
char *aeGetApiName(void);
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforeSleep);

#endif //INTEREST_REMINDER_AE_H
