//
// Created by wangl on 8/29/2021.
//

#include <unistd.h>
#include "table_test.h"
#include "stdio.h"
#include "stdlib.h"

#define timeE 1
#define fileE 2
#define allE (timeE | fileE)

int main(int argc, char **argv){

    if (!(fileE & timeE)){

    }


    return 0;
}
