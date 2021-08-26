//
// Created by wangl on 8/26/2021.
//

#include "server.h"
#include <unistd.h>

iRServer server;

int main(int argc, char** argv){

    return 0;
}

void initServer(){
    server.pid = getpid();

}