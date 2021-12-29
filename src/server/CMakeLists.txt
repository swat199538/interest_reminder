cmake_minimum_required(VERSION 2.8)

PROJECT(IReminderServer)

add_executable(IReminderServer server.c)

target_link_libraries(IReminderServer zmalloc)
target_link_libraries(IReminderServer adlist)
target_link_libraries(IReminderServer ae)
target_link_libraries(IReminderServer anet)
target_link_libraries(IReminderServer util)
target_link_libraries(IReminderServer sds)
target_link_libraries(IReminderServer fort)

