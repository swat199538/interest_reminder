//
// Created by wangl on 8/29/2021.
//

#include <unistd.h>
#include "table_test.h"
#include "stdio.h"
#include "stdlib.h"
#include "../common/ae.h"
#include "../common/zmalloc.h"



int main(int argc, char **argv){

    size_t real_size = sizeof(Point);

    Point *point = (Point *)zmalloc(sizeof(Point));

    point->x = 1;
    point->y = 1;

    size_t pontSize;

    void *total_pont;

    total_pont = (char *)point - real_size;

    pontSize = *((size_t*)total_pont);

    printf("real size is %zu, %zu is pont struct size\n", real_size, pontSize);

    printf("used memory is %zu\n", zmalloc_used());

    printf("memory free\n");

    zfree(point);

    printf("used memory is %zu\n", zmalloc_used());

    return 0;
}
