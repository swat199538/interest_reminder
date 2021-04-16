#include <stdlib.h>
#include "main.h"
#include "string.h"
#include "stdio.h"

int main(int argc, char** argv){

    //start schedule task
    if (argc == 1){

        return 0;
    }

    struct iConfig config = {
            .rate = 0,
            .name = "",
            .d_time =0,
            .money =0,
            .p_day=0,
            .s_date=0
    };

    argc--; argv++;

    /* Parse command line options. */
    while (argc > 0){

        if (argc == 1 && !strcmp(argv[0], "show"))
        {


            //show list
        } else if (argc >= 2 && !strcmp(argv[0], "-n"))
        {
            argc--;argv++;
            config.name = argv[0];
        } else if (argc >= 2 && !strcmp(argv[0], "-m"))
        {
            argc--;argv++;
            char *ptr;
            config.money = strtod(argv[0], &ptr);
        } else if (argc >= 2 && !strcmp(argv[0], "-s"))
        {
            argc--;argv++;
            char *ptr;
            config.s_date = strtol(argv[0], &ptr, 32);
        } else if (argc >= 2 && !strcmp(argv[0], "-d"))
        {
            argc--;argv++;
            char *ptr;
            config.p_day = strtol(argv[0], &ptr, 32);
        } else if (argc >= 2 && !strcmp(argv[0], "-r"))
        {
            argc--;argv++;
            char *ptr;
            config.rate = strtod(argv[0], &ptr);
        } else if (argc >= 2 && !strcmp(argv[0], "-c")){
            argc--;argv++;
            char *ptr;
            config.d_time = strtol(argv[0], &ptr, 32);
        }
        else
        {
            fprintf(stderr, "Invalid argument %s \n", argv[0]);
            exit(1);
        }

        argc--; argv++;
    }

    char *err = "";

    if (check_config(config, err) == 0){
        fprintf(stderr, "%s", err);
        exit(1);
    }

    return 0;
}

int check_config(struct iConfig config, char* err){

    return 0;
}