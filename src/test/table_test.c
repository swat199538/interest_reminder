//
// Created by wangl on 8/29/2021.
//

#include "table_test.h"
#include "stdio.h"

int main(int argc, char **argv){

    char *str = "this is a str";

    int a = sizeof(str);
    int b = sizeof(*str);

    printf("a is %d\n", a);
    printf("b is %d\n", b);

    return 0;
}