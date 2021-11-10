// Created by wangl on 8/26/2021.
#define __USE_XOPEN
#define _XOPEN_SOURCE
#include <time.h>
#include "server.h"
#include <unistd.h>
#include "stddef.h"
#include "time.h"
#include "pthread.h"
#include "signal.h"
#include "stdlib.h"
#include "fcntl.h"
#include "../common/util.h"
#include "stdio.h"
#include "../common/zmalloc.h"
#include <unistd.h>
#ifdef __linux__
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include "../common/sds.h"
#endif

/* Anti-warning macro... */
#define IR_NOTUSED(V) ((void) V)

/* Log levels */
#define IR_DEBUG 0
#define IR_VERBOSE 1
#define IR_NOTICE 2
#define IR_WARNING 3

#define IR_ENCODING_RAW 0    /* Raw representation */
#define IR_ENCODING_INT 1    /* Encoded as integer */
#define IR_ENCODING_ZIPMAP 2 /* Encoded as zipmap */
#define IR_ENCODING_HT 3     /* Encoded as an hash table */
#define IR_REQUEST_MAX_SIZE (1024*1024*256)
#define IR_MAX_WRITE_PER_EVENT (1024*64)

typedef struct iRClient{
    int fd;
    sds querybuf;// char* convert sds?
    sds *argv;
    int argc;
    int bulken;
    time_t lastinteraction; /* time of the last interaction, used for timeout */
    time_t blockingto;
    list *reply;
    int sentlen;
} iRClient;
typedef void iRCommandProc(iRClient *c);
struct iRCommand{
    char *name;
    iRCommandProc *proc;
    int arity;
};

static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask);
static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);
static void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask);
//static void sendReplyToClientWritev(aeEventLoop *el, int fd, void *privdata, int mask);
void addCommand(iRClient *c);
void showCommand(iRClient *c);
void addReply(iRClient *c, sds msg);
int processCommand(iRClient *c);
static struct iRCommand* lookupCommand(char *name);

/*================================= Globals ================================= */
struct iRServer server;
static struct iRCommand cmdTab[] = {
        {"add", addCommand, 9},
        {"show", showCommand, 1},
};
/*============================ Utility functions ============================ */
static int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    IR_NOTUSED(eventLoop);
    IR_NOTUSED(id);
    IR_NOTUSED(clientData);

    printf("server cron launch...\n");

    return 1000;
}

static void freeClient(iRClient *c)
{
    aeDeleteFileEvent(server.el, c->fd, AE_READABLE);
    aeDeleteFileEvent(server.el, c->fd, AE_WRITEABLE);
    listRelease(c->reply);
    close(c->fd);
    zfree(c);
}

static iRClient *createClient(int fd)
{
    iRClient *client = zmalloc(sizeof(struct iRClient));

    anetNonBlock(NULL, fd);
    anetNonDelay(NULL, fd);
    if (!client) return NULL;
    client->fd = fd;
    client->querybuf = sdsempty();
    client->argc = 0;
    client->reply = listCreate();
    client->sentlen = 0;
//    client->mbargc = 0;
    client->lastinteraction = time(NULL);

    if ((aeCreateFileEvent(server.el, client->fd, AE_READABLE,
       readQueryFromClient, client)) == AE_ERR){
        freeClient(client);
        return NULL;
    }

    listAddNodeTail(server.client, client);
    return client;
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

static void oom(const char *msg)
{
    iRLog(IR_WARNING, "%s out of memory\n", msg);
    sleep(1);
    abort();
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
    server.maxClients = 5;
    server.projectCount = 0;
}

void initServer()
{
    server.pid = getpid();
    server.thread_id = pthread_self();
    server.el = aeCreateEventLoop();
    server.client = listCreate();
    server.fd = anetTcpServer(server.neterr, server.port, server.bindaddr);
    if (server.fd == -1){
        iRLog(IR_WARNING, "Opening TCP port: %s", server.neterr);
        exit(1);
    }

    aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL);
    if (aeCreateFileEvent(server.el, server.fd, AE_READABLE, acceptHandler, NULL) == AE_ERR)
        oom("create file event");
}

static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask)
{
    int cport, cfd;
    char cip[128];
    iRClient *c;
    IR_NOTUSED(el);
    IR_NOTUSED(privdata);
    IR_NOTUSED(mask);

    cfd = anetAccept(server.neterr, fd, cip, &cport);

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

    if (server.maxClients && listLength(server.client) > server.maxClients){
        char *err = "-ERR max number of clients reached\r\n";
        write(c->fd, err, strlen(err));
        freeClient(c);
        return;
    }

    server.stat_numconnections++;
}

static void processInputBuff(iRClient *c){
    char *p;
again:
    p = strchr(c->querybuf, '\n');
    size_t querylen;

    if (p){
        sds query, *argv;
        int argc,j;

        query = c->querybuf;
        c->querybuf = sdsempty();
        querylen = 1 + (p-query);
        if (sdslen(query) > querylen){
            c->querybuf = sdscatlen(c->querybuf, query+querylen, sdslen(query)-querylen);
        }
        *p = '\0';
        if (*(p-1) == '\r') *(p-1) = '\0';
        sdsupdatelen(query);
        argv = sdssplitlen(query, sdslen(query), " ", 1, &argc);
        sdsfree(query);

        if (c->argv) zfree(c->argv);
        c->argv = zmalloc(sizeof(sds)*argc);

        for (j = 0; j < argc; j++) {
            if (sdslen(argv[j])){
                c->argv[j] = argv[j];
                c->argc++;
            } else{
                sdsfree(argv[j]);
            }
        }
        zfree(argv);
        if (c->argc){

        } else{
            if (sdslen(c->querybuf)) goto again;
        }
        return;
    } else if (sdslen(c->querybuf) >= IR_REQUEST_MAX_SIZE){
        iRLog(IR_VERBOSE, "client protocol err");
        freeClient(c);
        return;
    }
}

static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask){
    iRClient *c = (iRClient*) privdata;
    char buf[IR_IOBUF_LEN];
    int nread;
    IR_NOTUSED(el);
    IR_NOTUSED(mask);

    nread = read(fd, buf, IR_IOBUF_LEN);
    if (nread == -1){
        if (errno == EAGAIN){
            nread = 0;
        } else{
            iRLog(IR_VERBOSE, "Reading from client: %s", strerror(errno));
            freeClient(c);
            return;
        }
    } else if (nread == 0){
        iRLog(IR_VERBOSE, "Client closed connect");
        freeClient(c);
        return;
    }

    if (nread){
        c->querybuf = sdscatlen(c->querybuf, buf, sizeof(buf));
        c->lastinteraction = time(NULL);
    } else{
        return;
    }
    processInputBuff(c);
}

static void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask){
    iRClient *c = privdata;
    int nwritten=0, totwritten=0,objlen;
    sds o;
    IR_NOTUSED(el);
    IR_NOTUSED(mask);

    while (listLength(c->reply)){
        o = listNodeValue(listFirst(c->reply));
        objlen = sdslen(o);

        if(objlen == 0){
            listDelNode(c->reply, listFirst(c->reply));
            continue;
        }

        nwritten = write(fd, o + c->sentlen, objlen - c->sentlen);
        if (nwritten <= 0) break;

        c->sentlen += nwritten;
        totwritten += nwritten;

        if (c->sentlen == objlen){
            listDelNode(c->reply, listFirst(c->reply));
            c->sentlen = 0;
        }

        if (totwritten > IR_MAX_WRITE_PER_EVENT) break;
    }

    if (nwritten <= -1){
        if (errno == EAGAIN){
            nwritten = 0;
        } else{
            iRLog(IR_VERBOSE, "Error writting to client: %s", strerror(errno));
            freeClient(c);
            return;
        }
    }
    if (totwritten > 0) c->lastinteraction = time(NULL);
    if (listLength(c->reply) == 0){
        c->sentlen = 0;
        aeDeleteFileEvent(server.el, c->fd, AE_WRITEABLE);
    }
}

static void beforeSleep(struct aeEventLoop *eventLoop){
    IR_NOTUSED(eventLoop);
}

void serverLogFromHandler(int level, const char *msg){
    int fd;
    int log_to_stdout = server.logfile[0] == '\0';
    char buf[64];

    if ((level & 0xff) < server.verbosity || (log_to_stdout && server.daemonize))
        return;
    fd = log_to_stdout ? STDOUT_FILENO :
                        open(server.logfile, O_APPEND|O_WRONLY, 0644);
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

void addReply(iRClient *c, sds msg){
    if (listLength(c->reply) == 0 &&
        aeCreateFileEvent(server.el, c->fd,
        AE_WRITEABLE, sendReplyToClient, c) == AE_ERR) return;

    listAddNodeTail(c->reply, msg);
}

int processCommand(iRClient *c){
    struct iRCommand *cmd;

    if (!strcasecmp(c->argv[0], "quit")){
        freeClient(c);
        return 0;
    }

    cmd = lookupCommand(c->argv[0]);

    if (!cmd){
        addReply(c,
                 sdscatprintf(sdsempty(), "-ERR unknown command '%s'\r\n",
                              c->argv[0]));
        return 1;
    }


}

static struct iRCommand* lookupCommand(char *name){
    int j = 0;
    while (cmdTab[j].name != NULL){
        if (!strcasecmp(name, cmdTab[j].name)) return &cmdTab[j];
        j++;
    }
    return NULL;
}

/*================================= Command ================================= */
static void replyClientErr(iRClient *c, inObj *o, sds msg){
    zfree(o);
    addReply(c, msg);
}

void addCommand(iRClient *c){

    inObj *obj;

    obj = zmalloc(sizeof(inObj));

    obj->tage = c->argv[1];
    obj->name = c->argv[2];
    obj->bank = c->argv[3];

    struct tm dtm,etm;

    if (strptime(c->argv[4], "%Y-%m-%d", &dtm) == NULL){
        replyClientErr(c, obj, sdsnew("deposit date err\r\n"));
        return;
    }

    obj->depositDate = mktime(&dtm);

    if (strptime(c->argv[5], "%Y-%m-%d", &etm) == NULL){
        replyClientErr(c, obj, sdsnew("expiration date err\r\n"));
        return;
    }
    obj->expirationDate = mktime(&etm);

    if (sscanf(c->argv[6], "%zu", &obj->amount) == 0){
        replyClientErr(c, obj, sdsnew("amount err\r\n"));
        return;
    }

    if (sscanf(c->argv[7], "%e", &obj->rate) == 0){
        replyClientErr(c, obj, sdsnew("rate err\r\n"));
        return;
    }

    if (sscanf(c->argv[8], "%d", &obj->payoutDay) == 0){
        replyClientErr(c, obj, sdsnew("play out day err\r\n"));
        return;
    }

    server.project[server.projectCount] = obj;
    server.projectCount++;
}
/*================================= Command ================================= */

/*================================= Main ================================= */
int main(int argc, char** argv){
    time_t start;
    initServerConfig();
    initServer();
    start = time(NULL);
    aeSetBeforeSleepProc(server.el, beforeSleep);
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
    printf("%d", start);
    return 0;
}
