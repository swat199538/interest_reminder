cmake_minimum_required(VERSION 2.8)

PROJECT(IReminderServer)

add_executable(IReminderServer server.c ../common/cJSON.c ../common/util.c ../common/config.h ../common/anet.c ../common/anet.h)

