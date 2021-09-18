//
// Created by wangl on 8/29/2021.
//
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv){

    char *bind_addr = "127.0.0.1";

    int s, on = 1;
    struct sockaddr_in sa;


    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("create socket fail\n");
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
        printf("set opt fail\n");
        close(s);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1908);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind_addr){
        if ( inet_aton(bind_addr, &sa.sin_addr) == 0 ){
            printf("invalid addr error\n");
            close(s);
        }
    }

    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1){
        printf("bind addr fail\n");
        close(s);
    }

    if (listen(s, 511) == -1){
        printf("listen %d fail\n", s);
        close(s);
    }

    int fd; //accept fd
    struct sockaddr_in sa2;
    unsigned int sa2Len;

    sa2Len = sizeof(sa2Len);
    fd = accept(s, (struct sockaddr*)&sa2, &sa2Len);
    if (fd == -1){
        printf("accept error\n");
        close(s);
    }

    char *msg = "hello socket\n";

    write(fd, msg, strlen(msg) -1);

    for (int i = 0; i < 1000; ++i) {
        char string[32] ;
        sprintf(string,"%d", i) ;
        write(fd, string, strlen(string)-1);
    }

    close(s);

    return 0;
}

