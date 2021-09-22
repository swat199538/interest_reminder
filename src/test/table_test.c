//
// Created by wangl on 8/29/2021.
//
#include <stdio.h>
#include <stdlib.h>

#define AE_NONE 0
#define AE_READABLE 1

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

    eventLoop1 = malloc(sizeof(eventLoop));

    eventLoop1->mask = 1;

    int b;

    b = eventLoop1->mask & ~1;

    printf("%d\n", b);

    return 0;
}

