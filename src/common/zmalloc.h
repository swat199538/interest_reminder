//
// Created by swat on 9/8/2021.
// This code is copy to redis zmalloc
//

#ifndef INTEREST_REMINDER_ZMALLOC_H
#define INTEREST_REMINDER_ZMALLOC_H

#include "stddef.h"

void *zmalloc(size_t size);
void zfree(void *ptr);
size_t zmalloc_used(void );

#endif //INTEREST_REMINDER_ZMALLOC_H
