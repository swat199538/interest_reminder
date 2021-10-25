cmake_minimum_required(VERSION 2.8)

PROJECT(IReminderServer)

set(IR_SERVER_LIST ../common/adlist.c ../common/ae.c ../common/anet.c server.c ../common/zmalloc.c ../common/util.c ../common/sds.c)

add_executable(IReminderServer ${IR_SERVER_LIST})

