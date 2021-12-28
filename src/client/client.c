#include "../common/fmacros.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../common/zmalloc.h"
#include "../common/anet.h"
#include "../common/sds.h"

#define IR_CMD_INLINE 1
#define IR_CMD_BULK 2


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

struct redisCommand cmdTable[] = {
    {"show", 1, IR_CMD_INLINE},
    {NULL, 0, IR_CMD_INLINE}
};

static void usage(){
    printf("usage ir_cli -h[]\n");
    exit(1);
}

static struct redisCommand *lookupCommand(const char *name){
    int j = 0;

    while (cmdTable[j].name != NULL){
        if(!strcasecmp(cmdTable[j].name, name)) return &cmdTable[j];
        j++;
    }

    return NULL;
}

static int cliConnect(void){
    char err[ANET_ERR_LEN];
    static int fd = ANET_ERR_LEN;

    if(fd == ANET_ERR){
        fd = anetTcpConnect(err, config.host_ip, config.host_port);
        if(fd == ANET_ERR){
            fprintf()
        }

    }

}

static int cliSendCommand(int argc, char **argv){
    struct redisCommand *rc = lookupCommand(argv[0]);
    int fd, j, retval = 0;
    int read_forever = 0;
    sds cmd;

    if(!rc){
        fprintf(stderr, "Unknown command %s", argv[0]);
        return 1;
    }

    if( (rc->arity > 0 && argc != rc->arity) ||
        (rc->arity < 0 && argc < -rc->arity) ){
        fprintf(stderr, "Wrong number of  argumensts  for %s\n", rc->name);
        return 1;
    }




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
        cliSendCommand(argc, args);
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
