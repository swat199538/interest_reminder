cmake_minimum_required(VERSION 2.8)

PROJECT(ITest)

#add_executable(ITest table_test.c ../common/ae.c ../common/ae.h ../common/ae_epoll.c ../common/zmalloc.c ../common/zmalloc.h)
add_executable(ITest table_test.c ../common/zmalloc.c ../common/ae.c)

