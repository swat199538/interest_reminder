cmake_minimum_required(VERSION 2.8)

PROJECT(IReminderClient)

add_executable(IReminderClient client.c ../common/cJSON.c)