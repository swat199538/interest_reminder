#include <stddef.h>

static struct config{
    char *host_ip;
    int host_port;
    long repeat;
    int interactive;
    char *auth;
} config;

struct redisCommand{
    char *name;
    int arity;
    int flags;
};

static int parseOptions(int argc, char **argv){
    int i;

    for(i=1; i < argc; i++){

        int lastarg = i ==argc - 1;




    }

    return i;
}

int main(int argc, char** argv){

    int firstarg;
    char **argvcopy;
    struct redisCommand *rc;

    config.host_ip = "127.0.0.1";
    config.host_port = 9538;
    config.repeat = 1;
    config.interactive = 0;
    config.auth = NULL;

    return 0;
}
