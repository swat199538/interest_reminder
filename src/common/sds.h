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

#endif //__SDS_H
