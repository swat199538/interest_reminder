//
// Created by swat on 10/25/2021.
//
#define SDS_ABORT_ON_OOM

#include "sds.h"
#include "zmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void sdsOomAbort(void){
    fprintf(stderr,"SDS: Out Of Memory (SDS_ABORT_ON_OOM defined)\n");
    abort();
}

sds sdsnewlen(const void *init, size_t initlen){
    struct sdshdr *sh;

    sh = zmalloc(sizeof(struct sdshdr) + initlen +1);

#ifdef SDS_ABORT_ON_OOM
    if (sh == NULL) sdsOomAbort();
#else
    if (sh == NULL) return NULL;
#endif

    sh->len = initlen;
    sh->free = 0;
    if(initlen){
        if(init) memcpy(sh->buf, init, initlen);
        else memset(sh->buf, 0, initlen);
    }

    sh->buf[initlen] = '\0';
    return (char*) sh->buf;
}

sds sdsnew(const void *init){
    size_t len = (init == NULL) ? 0 : strlen(init);
    return sdsnewlen(init, len);
}

sds sdsempty(){
    return sdsnewlen("", 0);
}

size_t sdslen(const sds s){
   return ((struct sdshdr*)(s - sizeof(struct sdshdr)))->len;
}

sds sdsdup(const sds s){
    return sdsnewlen(s, sdslen(s));
}

void sdsfree(sds s){
    if (s == NULL) return;
    zfree(s - sizeof(s));
}

size_t sdsavail(const sds s){
    struct sdshdr *sh = (struct sdshdr*)(s - sizeof(s));
    return sh->len;
}