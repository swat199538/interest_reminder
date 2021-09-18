//
// Created by wangl on 8/29/2021.
//
#include <stdio.h>

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

typedef struct eventLoop{
    int mask;
}eventLoop;

static void apiDelEvent(eventLoop *eventLoop, int fd, int delmask){
    int mask = eventLoop->mask & (~ delmask);
}

static void aeDeleteFileEvent(eventLoop *eventLoop1, int fd, int mask){
    eventLoop1->mask = eventLoop1->mask & (~mask);
}

int main(int argc, char **argv){

    eventLoop *eventLoop1;

    eventLoop1->mask

    return 0;
}

