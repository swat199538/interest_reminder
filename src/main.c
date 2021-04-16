#include <stdlib.h>
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "cJSON.h"

int main(int argc, char** argv){

    struct iConfig config = {
            .rate = 0,
            .name = "",
            .d_time =0,
            .money =0,
            .p_day=0,
            .s_date=0
    };

    //start schedule task
    if (argc == 1){
        char* err;
        save_config(config, err);
        return 0;
    }



    argc--; argv++;

    /* Parse command line options. */
    while (argc > 0){

        if (argc == 1 && !strcmp(argv[0], CL_SHOW))
        {


            //show list
        } else if (argc >= 2 && !strcmp(argv[0], CL_NAME))
        {
            argc--;argv++;
            config.name = argv[0];
        } else if (argc >= 2 && !strcmp(argv[0], CL_MONEY))
        {
            argc--;argv++;
            char *ptr;
            config.money = strtod(argv[0], &ptr);
        } else if (argc >= 2 && !strcmp(argv[0], CL_S_DATE))
        {
            argc--;argv++;
            struct tm tm = {0};

            int year, month, day;

            if (sscanf(argv[0], "%d-%d-%d", &year, &month, &day) == 3){
                tm.tm_year = year - 1900;
                tm.tm_mon = month -1;
                tm.tm_mday = day;
            } else{
                fprintf(stderr, "Invalid argument %s \n", argv[0]);
                exit(1);
            }

            config.s_date = tm;
        } else if (argc >= 2 && !strcmp(argv[0], CL_P_DAY))
        {
            argc--;argv++;
            char *ptr;
            config.p_day = strtol(argv[0], &ptr, 32);
        } else if (argc >= 2 && !strcmp(argv[0], CL_RATE))
        {
            argc--;argv++;
            char *ptr;
            config.rate = strtod(argv[0], &ptr);
        } else if (argc >= 2 && !strcmp(argv[0], CL_D_TIME)){
            argc--;argv++;
            char *ptr;
            config.d_time = strtol(argv[0], &ptr, 32);
        } else
        {
            fprintf(stderr, "Invalid argument %s \n", argv[0]);
            exit(1);
        }

        argc--; argv++;
    }

    char *err = "";

    if (check_config(config, err) == 1){
        fprintf(stderr, "%s", err);
        exit(1);
    }

    return 0;
}

/**
 * check config
 */
int check_config(struct iConfig config, char* err){

    if (!strcmp(config.name, "")){
        sprintf(err, "Invalid argument %s value \n", CL_NAME);
        return 1;
    }

    if (config.rate == 0){
        sprintf(err, "Invalid argument %s value \n", CL_NAME);
        return 1;
    }

    if (config.d_time == 0){
        sprintf(err, "Invalid argument %s value \n", CL_NAME);
        return 1;
    }

    if (config.money == 0){
        sprintf(err, "Invalid argument %s value \n", CL_MONEY);
        return 1;
    }

    if (config.p_day == 0){
        sprintf(err, "Invalid argument %s value \n", CL_P_DAY);
        return 1;
    }

    if (config.s_date.tm_year == 0){
        sprintf(err, "Invalid argument %s value \n", CL_S_DATE);
        return 1;
    }

    return 0;
}

/**
 * save config
 */
int save_config(struct  iConfig config, char* err){

    cJSON* pRoot = cJSON_CreateObject();

    cJSON* pItem = cJSON_CreateObject();

    cJSON_AddStringToObject(pItem, "name", "config.name");
    cJSON_AddItemToObject(pRoot, "0", pItem);

    char* json = cJSON_Print(pRoot);

    printf("%s", json);

    return 0;
}