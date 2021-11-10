//
// Created by swat on 10/25/2021.
//

#ifndef __SDS_H
#define __SDS_H

#include <sys/types.h>

typedef char *sds;

struct sdshdr{
    long len;
    long free;
    char buf[];
};

sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const void *init);
sds sdsempty();
size_t sdslen(const sds s);
sds sdsdup(const sds s);
void sdsfree(sds s);
size_t sdsavail(const sds s);
sds sdscatlen(sds s, void *t, size_t len);
sds sdscat(sds s, char *t);
sds sdscpylen(sds s, char *t, size_t len);
void sdsupdatelen(sds s);
sds *sdssplitlen(char *s, int len, char *sep, int seplen, int *count);

#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...);
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

#endif //__SDS_H
