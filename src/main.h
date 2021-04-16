#include <time.h>

#ifndef INTEREST_REMINDER_MAIN_H
#define INTEREST_REMINDER_MAIN_H

#endif //INTEREST_REMINDER_MAIN_H

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


int check_config(struct iConfig config, char* err);