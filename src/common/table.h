//
// Created by wangl on 8/28/2021.
//

#ifndef INTEREST_REMINDER_TABLE_H
#define INTEREST_REMINDER_TABLE_H

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "string.h"

typedef struct Table{
    size_t row_num;
    size_t col_num;
    size_t *col_max_width;
    char ***content;
} table;

extern void format_table(table *t, char* *formatted_out)

#endif //INTEREST_REMINDER_TABLE_H
