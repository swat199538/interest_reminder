// Created by wangl on 8/26/2021.

#include "server.h"
#include <unistd.h>
#include "stddef.h"
#include "time.h"
#include "pthread.h"
#include "signal.h"
#include "stdlib.h"
#include "fcntl.h"
#include "../common/util.h"
#include "string.h"
#include "stdio.h"
#ifdef __linux__
#include <sys/mman.h>
#include <stdarg.h>

#endif

/* Anti-warning macro... */
#define IR_NOTUSED(V) ((void) V)

/* Log levels */
#define IR_DEBUG 0
#define IR_VERBOSE 1
#define IR_NOTICE 2
#define IR_WARNING 3

static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask);

typedef struct iRClient{
    int fd;
} iRClient;

/*================================= Globals ================================= */
struct iRServer server;
/*============================ Utility functions ============================ */
static int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    IR_NOTUSED(eventLoop);
    IR_NOTUSED(id);
    IR_NOTUSED(clientData);

    printf("server cron launch...\n");

    return 1000;
}

static iRClient *createClient(int fd)
{
    return NULL;
}

static void freeClient(iRClient *c)
{

}

static void iRLog(int level, const char *fmt, ...)
{
    va_list ap;
    FILE *fp;

    fp = (server.logfile == NULL) ? stdout : fopen(server.logfile, "a");
    if (!fp) return;

    va_start(ap, fmt);
    if (level >= server.verbosity){
        char *c = ".-*#";
        char buf[64];
        time_t now;

        now = time(NULL);
        strftime(buf,64,"%d %b %H:%M:%S",localtime(&now));
        fprintf(fp,"[%d] %s %c ",(int)getpid(),buf,c[level]);
        vfprintf(fp, fmt, ap);
        fprintf(fp,"\n");
        fflush(fp);
    }
    va_end(ap);
    if (server.logfile) fclose(fp);
}

static void sigShutdownHandler(int sig){
    char *msg;

    switch (sig) {
        case SIGINT:
            msg = "Received SIGINT scheduling shutdown ...";
            break;
        case SIGTERM:
            msg = "Received SIGTERM scheduling shutdown ...";
            break;
        default:
            msg = "Received shutdown signal, scheduling shutdown ...";
            break;
    }

    if (server.shutdown_asap && sig == SIGINT){
        serverLogFromHandler(LL_WARNING, "You insist... exiting now.");
        //todo: del tmp file
        exit(1);
    } else if (server.loading){
        serverLogFromHandler(LL_WARNING, "Received shutdown signal during loading, exiting now.");
        exit(0);
    }

    serverLogFromHandler(LL_WARNING, msg);
    server.shutdown_asap = 1;
}

void initServerConfig(){
    server.port = IR_SERVER_PORT;
    server.bindaddr = IR_SERVER_BIND_ADDR;
}

void initServer(){
    server.pid = getpid();
    server.thread_id = pthread_self();
    server.el = aeCreateEventLoop();
    server.fd = anetTcpServer(server.neterr, server.port, server.bindaddr);

    if (server.fd == -1){
        iRLog(IR_WARNING, "Opening TCP port: %s", server.neterr);
        exit(1);
    }

    aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL);


}

static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask)
{
    int cport, cfd;
    char cip[128];
    iRClient *c;
    IR_NOTUSED(el);
    IR_NOTUSED(privdata);
    IR_NOTUSED(mask);

    cfd = anetAccept(server.neterr, fd, &cip, &cport);

    if (cfd == ANET_ERR){
        iRLog(IR_VERBOSE, "Accepting client connection: %s", server.neterr);
        return;
    }

    iRLog(IR_VERBOSE,"Accepted %s:%d", cip, cport);

    if ((c = createClient(cfd)) == NULL){
        iRLog(IR_WARNING,"Error allocating resoures for the client");
        close(cfd);
        return;
    }

    server.stat_numconnections++;
}

void setupSignalHandlers(void){
    struct sigaction act;

    sigemptyset(&act.sa_mask);

    act.sa_flags = 0;
    act.sa_handler = sigShutdownHandler;

}

void serverLogFromHandler(int level, const char *msg){
    int fd;
    int log_to_stdout = server.logfile[0] == '\0';
    char buf[64];

    if ((level & 0xff) < server.verbosity || (log_to_stdout && server.daemonize))
        return;
    fd = log_to_stdout ? STDOUT_FILENO :
                        open(server.logfile, O_APPEND|O_CLOEXEC|O_WRONLY, 0644);
    if (fd == -1) return;

    ll2string(buf, sizeof(buf), getpid());
    if (write(fd, buf, strlen(buf)) == -1) goto err;
    if (write(fd,":signal-handler (",17) == -1) goto err;
    ll2string(buf,sizeof(buf),time(NULL));
    if (write(fd,buf,strlen(buf)) == -1) goto err;
    if (write(fd,") ",2) == -1) goto err;
    if (write(fd,msg,strlen(msg)) == -1) goto err;
    if (write(fd,"\n",1) == -1) goto err;
err:
    if (!log_to_stdout) close(fd);
}

/*================================= Main ================================= */
int main(int argc, char** argv){

    time_t start;

    initServerConfig();

    initServer();

    start = time(NULL);

    printf("%d", start);

    return 0;
}
