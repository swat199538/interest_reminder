//
// Created by swat on 9/23/2021.
//
#include "anet.h"
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

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