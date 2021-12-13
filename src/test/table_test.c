//
// Created by wangl on 8/29/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_BUFF_SIZE 1024
#define FD_NONBLOCK 1

static struct config{
    int a;
    char *b;
} config;

static int set_nonblock(int fd){
    int old_flg = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_flg | O_NONBLOCK);
    return old_flg;
}

static void addfd_to_epoll(int epoll_fd, int fd, int epoll_type, int block_type){
    struct epoll_event ep_event;
    ep_event.data.fd = fd;
    ep_event.events = EPOLLIN;

    if (epoll_type == EPOLLET)
        ep_event.events |= EPOLLET;

    if (block_type == FD_NONBLOCK)
        set_nonblock(fd);

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ep_event);
}

static void epoll_et_loop(int sock_fd){
    char buffer[MAX_BUFF_SIZE];
    int ret;

    while (1){
        memset(buffer, 0, MAX_BUFF_SIZE);
        ret = recv(sock_fd, buffer, MAX_BUFF_SIZE, 0);
        if (ret == -1){

            if (errno == EAGAIN || errno == EWOULDBLOCK){
                printf("loop read all data\n");
                break;
            }
            close(sock_fd);
            break;
        } else if (ret == 0){
            printf("client close connect\n");
            close(sock_fd);
        } else{
            printf("receive msg:%s, count %d size\n", buffer, ret);
        }
    }
    printf("epoll end\n");
}

static void epoll_process(int epoll_fd, struct epoll_event *events, int number, int socket_fd, int epoll_type,
        int block_type)
{
    struct sockaddr_in client_addr;
    socklen_t client_addrlen;
    int newfd, connfd;
    int i;

    for (i = 0; i < number; ++i) {
        newfd = events[i].data.fd;
        //这是一个socket，处理accept
        if (newfd == socket_fd){
            printf("====================== new accept =======================\n");
            printf("accept() start\n");
            printf("sleep 3s\n");
            sleep(3);
            printf("sleep end\n");
            client_addrlen = sizeof(client_addr);
            connfd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addrlen);
            printf("connfd = %d\n", connfd);
            addfd_to_epoll(epoll_fd, connfd, epoll_type, 1);
            printf("accept() end\n");
        } else if (events[i].events & EPOLLIN){
            epoll_et_loop(newfd);
        } else{
            printf("other events trigger\n");
        }
    }
}

void err_exit(char *msg){
    perror(msg);
    exit(1);
}

int create_socket(const char *ip, const int port_number){
    struct sockaddr_in sever_addr;
    int socktfd, reuse = 1;

    memset(&sever_addr, 0, sizeof(sever_addr));
    sever_addr.sin_family = AF_INET;
    sever_addr.sin_port = htons(port_number);

    if (inet_pton(PF_INET, ip, &sever_addr.sin_addr) == -1)
        err_exit("inet_pton error\n");

    if ((socktfd = socket(PF_INET, SOCK_STREAM, 0) ) == -1)
        err_exit("socket error\n");

    if (setsockopt(socktfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1 )
        err_exit("set opt error\n");

    if (bind(socktfd, (struct sockaddr*)&sever_addr, sizeof(sever_addr)) == -1)
        err_exit("bind fail\n");

    if (listen(socktfd, 5) == -1)
        err_exit("listen fail\n");

    return socktfd;
}

static void m_print(){
    printf("%d\n", config.a);
    printf("%s\n", config.b);
}

int main(int argc, char **argv){

//    int socket_fd, epoll_fd, number;
//
//    socket_fd = create_socket("127.0.0.1", atoi("5050"));
//
//    struct epoll_event events[20];
//
//    if ((epoll_fd = epoll_create1(0)) == -1)
//        err_exit("epoll creat fail\n");
//
//    addfd_to_epoll(epoll_fd, socket_fd, 1, FD_NONBLOCK);
//
//    while (1){
//        number = epoll_wait(epoll_fd, events, 5, 10);
//
//        if (number == 0){
//            printf("no ready event\n");
//        }else if (number == -1)
//            err_exit("epoll wait fail");
//        else{
//            epoll_process(epoll_fd, events, number, socket_fd, 0, FD_NONBLOCK);
//        }
//    }
//
//    close(socket_fd);
//    return 0;


}

