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
#include "../common/fort.h"
#include <unistd.h>
#ifdef __linux__
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
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
#define IR_MAX_PROJECT_COUNT 1024

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
static void addCommand(iRClient *c);
static void showCommand(iRClient *c);
static void showConsoleCommand(iRClient *c);
void addReply(iRClient *c, sds msg);
int processCommand(iRClient *c);
static struct iRCommand* lookupCommand(char *name);
static void resetClient(iRClient *c);
static void call(iRClient *c, struct iRCommand *cmd);
static void  stopCommand(iRClient *c);

/*================================= Globals ================================= */
struct iRServer server;
static struct iRCommand cmdTab[] = {
        {"add",   addCommand,         9},
        {"show",  showCommand,        1},
        {"showc", showConsoleCommand, 1},
        {"stop",  stopCommand,        1},
};
/*============================ Utility functions ============================ */
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
    client->argv = NULL;
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

static int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    IR_NOTUSED(eventLoop);
    IR_NOTUSED(id);
    IR_NOTUSED(clientData);

    if (server.projectCount > 0){
        ft_table_t *table = ft_create_table();
        ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ANY_ROW);
        ft_write_ln(table, "tag", "name", "bank", "amount", "rate");
        int j;
        for (j=0; j < server.projectCount; j++){
            char amountStr[256], rateStr[256];
            memset(amountStr, 0, 256);
            memset(rateStr, 0, 256);
            snprintf(amountStr, sizeof(amountStr), "%zu", server.project[j]->amount);
            snprintf(rateStr, sizeof(rateStr), "%f", server.project[j]->rate);
            ft_write_ln(table, server.project[j]->tag, server.project[j]->name, server.project[j]->bank,
                        amountStr, rateStr);
        }
        fprintf(stdout, "%s", ft_to_string(table));
        fflush(stdout);
        ft_destroy_table(table);
    }

    return 10000;
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
    server.project = zmalloc(sizeof(inObj)* IR_MAX_PROJECT_COUNT);
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
            if (processCommand(c) && sdslen(c->querybuf)) goto again;
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
        c->querybuf = sdscatlen(c->querybuf, buf, nread);
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
        addReply(c,sdscatprintf(sdsempty(), "-ERR unknown command '%s'\r\n",
                          c->argv[0]));
        resetClient(c);
        return 1;
    } else if (cmd->arity > 0 && cmd->arity != c->argc || c->argc < -cmd->arity){
        addReply(c, sdscatprintf(sdsempty(), "-ERR wrong number of arguments for '%s' command\r\n",
                           cmd->name));
        resetClient(c);
        return 1;
    }

    call(c, cmd);
    resetClient(c);
    return 1;
}

static void resetClient(iRClient *c){
    int j;
    for(j=0; j < c->argc; j++){
        sdsfree(c->argv[j]);
    }
    c->argc = 0;
}

static void call(iRClient *c, struct iRCommand *cmd){
    cmd->proc(c);
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

static void addCommand(iRClient *c){

    if (server.projectCount >= IR_MAX_PROJECT_COUNT){
        addReply(c, "add error beyond max number\n");
        return;
    }

    inObj *obj;

    obj = zmalloc(sizeof(inObj));

    obj->tag = sdscat(sdsempty(), c->argv[1]);
    obj->name = sdscat(sdsempty(), c->argv[2]);
    obj->bank = sdscat(sdsempty(), c->argv[3]);

    struct tm dtm,etm;
    memset(&dtm, 0, sizeof(dtm));
    memset(&etm, 0, sizeof(etm));

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

static time_t refineDate(time_t t){
    struct tm *tmt = localtime(&t);
    tmt->tm_hour = 0;
    tmt->tm_min = 0;
    tmt->tm_sec = 0;

    time_t value = mktime(tmt);
    return value;
}

static float calculateGrossInterest(inObj *obj, float *rest, float *play){

    float total = 0;
    float played = 0;

    size_t s = 60 * 60 * 24 * obj->payoutDay;
    time_t pt = obj->depositDate + s;
    time_t today = time(NULL);

    while (refineDate(pt) < refineDate(obj->expirationDate)){
        total += obj->amount * obj->rate;
        if (refineDate(pt) <= refineDate(today)){
            played +=  obj->amount * obj->rate;
        }
        time_t tmp = pt +s;
        if (refineDate(pt + s) >= refineDate(obj->expirationDate)){
            break;
        }
        pt+=s;
    }

    if (refineDate(obj->expirationDate) > refineDate(pt)){
        int day =( refineDate(obj->expirationDate) - refineDate(pt))/(24*3600);
        total += (obj->amount * obj->rate) / obj->payoutDay * day;
    }

    *play = played;
    *rest = total - played;

    return total;
}

static void st_to_str(size_t t, char *t_str, int len){
    memset(t_str, 0, 256);
    snprintf(t_str, len, "%zu", t);
}

static void f_to_str(float f, char *t_str, int len){
    memset(t_str, 0, len);
    snprintf(t_str, len, "%f", f);
}

static void t_to_str(time_t t, char *t_str, int len){
    memset(t_str, 0, len);
    struct tm *tmp = localtime(&t);
    strftime(t_str, len, "%Y-%m-%d", tmp);
}


static void showCommand(iRClient *c){
    ft_table_t *table = ft_create_table();

    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ANY_ROW);
    ft_write_ln(table, "tag", "name", "bank", "amount", "rate", "deposit time", "expiration time",
                "payout cycle(day)", "total");

    //, "gross interest", "interest earned", "the residual interest",
    //                "next pay date"

    int j;
    for (j = 0; j < server.projectCount; ++j) {
        inObj *o = server.project[j];

        float rest, total, play;
        total = calculateGrossInterest(o, &rest, &play);

        char dd[80], ed[80], podStr[255], amountStr[80], rateStr[80], totalStr[255];
        t_to_str(o->depositDate, dd, 80);
        t_to_str(o->expirationDate, ed, 80);
        st_to_str(o->amount, amountStr, 80);
        f_to_str(o->rate, rateStr, 80);
        st_to_str(o->payoutDay, podStr, 255);
        f_to_str(total, totalStr, 255);

        ft_write_ln(table, o->tag, o->name, o->bank, amountStr, rateStr, dd, ed, podStr, totalStr);
    }
    addReply(c, sdscatprintf(sdsempty(), "%s", ft_to_string(table)));
    ft_destroy_table(table);
}

static void showConsoleCommand(iRClient *c){
    ft_table_t *table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ANY_ROW);
    ft_write_ln(table, "tag", "name", "bank", "amount", "rate");
    int j;
    for (j=0; j < server.projectCount; j++){
        char amountStr[256], rateStr[256];
        memset(amountStr, 0, 256);
        memset(rateStr, 0, 256);
        snprintf(amountStr, sizeof(amountStr), "%zu", server.project[j]->amount);
        snprintf(rateStr, sizeof(rateStr), "%f", server.project[j]->rate);
        ft_write_ln(table, server.project[j]->tag, server.project[j]->name, server.project[j]->bank,
                    amountStr, rateStr);
    }
    addReply(c, sdscatprintf(sdsempty(), "%s", ft_to_string(table)));
    fprintf(stdout, "%s", ft_to_string(table));
    fflush(stdout);
    ft_destroy_table(table);
}

static void stopCommand(iRClient *c){
    IR_NOTUSED(c);
    aeStop(server.el);
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
