//
// Created by swat on 9/8/2021.
//

#include <stdlib.h>
#include "zmalloc.h"
#include <stdio.h>
#include <pthread.h>

#define PREFIX_SIZE sizeof(size_t)

#define increment_used_memory(_n) do{ \
   if (zmalloc_thread_safe){\
        pthread_mutex_lock(&used_memory_mutex);\
        used_memory += _n;\
        pthread_mutex_unlock(&used_memory_mutex);\
    } else{\
        used_memory += _n;\
    }\
} while(0)

#define decrement_used_memory(_n) do {\
    if (zmalloc_thread_safe){\
        pthread_mutex_lock(&used_memory_mutex);\
        used_memory -= _n;\
        pthread_mutex_unlock(&used_memory_mutex);\
    } else{\
        used_memory -= _n;\
    }\
} while (0)



static size_t used_memory = 0;
static int zmalloc_thread_safe = 0;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;


static void zmalloc_oom(size_t size){
    sprintf(stderr, "zmalloc: Out of memory trying to allocate %zu bytes\n",
            size);
    fflush(stderr);
    abort();
}

void *zmalloc(size_t size){
    void *ptr = malloc(size + PREFIX_SIZE);// malloc memory  is divided into recording areas and value areas
    if (!ptr) zmalloc_oom(size); // malloc failed abort program
    (*(size_t *)ptr) = size; //recode memory size
    increment_used_memory(size + PREFIX_SIZE);
    return (char *)ptr + PREFIX_SIZE;
}

void zfree(void *ptr){
    void *realPtr;
    size_t old_size;
    realPtr = (char *)ptr - PREFIX_SIZE;
    old_size = *((size_t *)realPtr);
    decrement_used_memory(old_size + PREFIX_SIZE);
    free(realPtr);
}

size_t zmalloc_used(){
    size_t m;
    if (zmalloc_thread_safe) pthread_mutex_lock(&used_memory_mutex);
    m = used_memory;
    if (zmalloc_thread_safe) pthread_mutex_unlock(&used_memory_mutex);
    return m;
}
