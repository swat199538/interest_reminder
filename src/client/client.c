#include "../common/fmacros.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../common/zmalloc.h"
#include "../common/anet.h"


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

static void usage(){
    printf("usage ir_cli -h[]\n");
    exit(1);
}

static int cliSendCommand(int argc, char **argv){

};

static int parseOptions(int argc, char **argv){
    int i;

    for(i=1; i < argc; i++){

        int lastarg = i ==argc - 1;
        if(!strcmp(argv[i], "-h") && !lastarg){
            char *ip = zmalloc(32);
            if(anetResolve(NULL, argv[i+1], ip) == ANET_ERR){
                printf("Can't resolve %s", argv[i]);
                exit(1);
            }
            config.host_ip = ip;
            i++;
        } else if (!strcmp(argv[i], "-h") && lastarg){
            usage();
        } else if( !strcmp(argv[i], "-p") && !lastarg){
            config.host_port = atoi(argv[i+1]);
            i++;
        } else{
            break;
        }
    }
    return i;
}

static char *prompt(char *line, int size){
    char *retval;

    do {
        printf(">> ");
        retval = fgets(line, size, stdin);
    } while (retval && *line == '\n');
    line[strlen(line) - 1] = '\0';

    return retval;
}

static void repl(){
    int size = 4096, max = size >> 1, argc;

    char buffer[size];
    char *line = buffer;
    char **ap, *args[max];

    while (prompt(line, size)) {
        argc = 0;

        for(ap = args; (*ap = strsep(&line, " \t")) == NULL;){
            if(**ap != '\0'){
                if(argc >= max) break;
                if(strcasecmp(*ap, "quit") == 0 || strcasecmp(*ap, "exit") == 0)
                    exit(0);
                ap++;
                argc++;
            }
        }

        config.repeat = 1;
        line = buffer;
    }

    exit(0);
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

    firstarg = parseOptions(argc, argv);

    argc -= firstarg;
    argv += firstarg;

    if(argc == 0 || config.interactive == 1) repl();

    return 0;
}
