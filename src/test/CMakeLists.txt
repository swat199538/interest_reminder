cmake_minimum_required(VERSION 2.8)

PROJECT(ITest)

add_executable(ITest table_test.c)

target_link_libraries(zmalloc)
target_link_libraries(ae)