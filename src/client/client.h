#include <time.h>

#ifndef INTEREST_REMINDER_MAIN_H
#define INTEREST_REMINDER_MAIN_H

#endif //INTEREST_REMINDER_MAIN_H

#define CL_NAME "-n"
#define CL_MONEY "-m"
#define CL_S_DATE "-s"
#define CL_P_DAY "-d"
#define CL_RATE "-r"
#define CL_D_TIME "-c"
#define CL_SHOW "show"

typedef struct iConfig{
    // The name of the account
    char * name;//

    // amount deposited
    double money;//

    // deposit interest rate
    double rate;

    // Deposit time(days)
    long d_time;

    // payment of interest duration(days)
    long p_day;//

    // start time
    struct tm s_date;//
} iConfig;

/**
 * check config with argument
 * @param config
 * @param err
 * @return
 */
int check_config(struct iConfig config, char* err);

/**
 * config save in file
 * @param config
 * @param err
 * @return
 */
int save_config(struct  iConfig config, char* err);