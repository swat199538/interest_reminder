//
// Created by swat on 9/23/2021.
//
#include "anet.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

static void anetSetError(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

int anetTcpServer(char *err, int port, char *addr)
{
    int s, on = 1;
    struct sockaddr_in sa;

    if ( (s = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        anetSetError(err, "socket: %s\n", strerror(errno));
        return ANET_ERR;
    }

    // When port is released, it can be immediately used once more.
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
        anetSetError(err, "setsockopt err: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (addr){
        if (inet_aton(addr, &sa.sin_addr) == 0){
            anetSetError(err, "Invalid bind address\n");
            close(s);
            return ANET_ERR;
        }
    }
    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1){
        anetSetError(err, "bind: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    if (listen(s, 511) == -1){
        anetSetError(err, "listen: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return s;
}

int anetAccept(char *err, int serverSock, char *ip, int *port)
{
    int fd;
    struct sockaddr_in sa;
    unsigned int saLen;

    while (1){
        saLen = sizeof(sa);
        fd = accept(serverSock, (struct sockaddr*)&sa, &saLen);
        if (fd == -1){
            if (errno == EINTR)
                continue;
            else{
                anetSetError(err, "accept: %s\n", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }

    if (ip) strcpy(ip, inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);

    return fd;
}

int anetNonBlock(char *err, int fd)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1){
        anetSetError(err, "fcntl(F_GETFL):%s\n", strerror(errno));
        return ANET_ERR;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        anetSetError(err, "fcntl(F_SETFL):%s\n", strerror(errno));
        return ANET_ERR;
    }

    return ANET_OK;
}

int anetNonDelay(char *err, int fd)
{
    int yes = 1;

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1){
        anetSetError(err, "setsockopt TCP_NODELAY: %s\n", strerror(errno));
        return ANET_ERR;
    }

    return ANET_OK;
}

int anetResolve(char *err, char *host, char *ipbuf)
{
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    if(inet_aton(host, &sa.sin_addr) == 0){
        struct hostent *he;

        he = gethostbyname(host);
        if(he == NULL){
            anetSetError(err, "can't resolve %s", host);
            return ANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr_list[0], sizeof(he));
    }
    strcpy(ipbuf, inet_ntoa(sa.sin_addr));
    return ANET_OK;
}