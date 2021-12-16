//
// Created by swat on 9/23/2021.
//

#ifndef INTEREST_REMINDER_ANET_H
#define INTEREST_REMINDER_ANET_H

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

int anetTcpServer(char *err, int port, char *addr);
int anetAccept(char *err, int serverSock, char *ip, int *port);
int anetNonBlock(char *err, int fd);
int anetNonDelay(char *err, int fd);
int anetResolve(char *err, char *host, char *ipbuf);

#endif //INTEREST_REMINDER_ANET_H
