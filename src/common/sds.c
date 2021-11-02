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

sds sdscat(sds s, char *t){
    return sdscatlen(s, t, strlen(t));
}

sds sdscpylen(sds s, char *t, size_t len){
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    size_t totlen = sh->len+sh->free;

    if(totlen < len){
        s = sdsMakeRoomFor(s, len-totlen);
        if(s == NULL) return NULL;
        sh = (void *)(s - sizeof(struct sdshdr));
        totlen = sh->len+sh->free;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen - len;
    return s;
}

void sdsupdatelen(sds s){
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

sds *sdssplitlen(char *s, int len, char *sep, int seplen, int *count) {
    int elements = 0, slots = 5, start = 0, j;

    sds *tokens = zmalloc(sizeof(sds)*slots);
#ifdef SDS_ABORT_ON_OOM
    if (tokens == NULL) sdsOomAbort();
#endif
    if (seplen < 1 || len < 0 || tokens == NULL) return NULL;
    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            sds *newtokens;

            slots *= 2;
            newtokens = zrealloc(tokens,sizeof(sds)*slots);
            if (newtokens == NULL) {
#ifdef SDS_ABORT_ON_OOM
                sdsOomAbort();
#else
                goto cleanup;
#endif
            }
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = sdsnewlen(s+start,j-start);
            if (tokens[elements] == NULL) {
#ifdef SDS_ABORT_ON_OOM
                sdsOomAbort();
#else
                goto cleanup;
#endif
            }
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = sdsnewlen(s+start,len-start);
    if (tokens[elements] == NULL) {
#ifdef SDS_ABORT_ON_OOM
        sdsOomAbort();
#else
        goto cleanup;
#endif
    }
    elements++;
    *count = elements;
    return tokens;

#ifndef SDS_ABORT_ON_OOM
    cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) sdsfree(tokens[i]);
        zfree(tokens);
        return NULL;
    }
#endif
}