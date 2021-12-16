cmake_minimum_required(VERSION 2.8)

PROJECT(IReminderClient)

set(__USE_MISC)

add_executable(ir_cli client.c ../common/anet.c ../common/zmalloc.c)