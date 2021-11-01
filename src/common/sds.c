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

static sds sdsMakeRoomFor(sds s, size_t addlen){
    struct sdshdr *sh, *newsh;
    size_t free = sdsavail(s);
    size_t len, newlen;
    if(free >= addlen) return s;
    len = sdslen(s);
    sh = (void *)s - sizeof(struct sdshdr);
    newlen = (len+addlen)*2;
    newsh = zrealloc(sh, sizeof(struct sdshdr)+newlen+1);
#ifdef SDS_ABORT_ON_OOM
    if (newsh == NULL) sdsOomAbort();
#else
    if(newsh == NULL) return NULL;
#endif
    newsh->free = newlen - len;
    return newsh->buf;
}

sds sdscatlen(sds s, void *t, size_t len){
    struct sdshdr *sh;
    size_t culen = sdslen(s);

    s = sdsMakeRoomFor(s, len);
    if(s == NULL) return NULL;
    sh = (struct sdshdr*)s - sizeof(struct sdshdr);
    memcpy(s+culen, t, len);
    sh->len = culen+len;
    sh->free = sh->free - len;
    //todo:test equal sh->buf[culen+len] = '\0';
    s[culen+len] = '\0';
    return s;
}