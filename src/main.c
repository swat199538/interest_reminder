#include <stdlib.h>
#include "main.h"
#include "string.h"
#include "stdio.h"

int main(int argc, char** argv){

    //start schedule task
    if (argc == 1){

        return 0;
    }

    argc--; argv++;

    /* Parse command line options. */
    while (argc > 0){

        if (argc == 1 && !strcmp(argv[0], "show")) {
            //show list
        } else if (argc >= 2 && !strcmp(argv[0], "-n")){

        } else if (argc >= 2 && !strcmp(argv[0], "-p")){

        } else if (argc >= 2 && !strcmp(argv[0], "-r")){

        } else if (argc >= 2 && !strcmp(argv[0], "-d")){

        } else if (argc >= 2 && !strcmp(argv[0], "-n")){

        } else{
            fprintf(stderr, "Invalid argument %s \n", argv[0]);
            exit(1);
        }

        argc--; argv++;
    }


    return 0;
}