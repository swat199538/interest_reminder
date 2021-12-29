cmake_minimum_required(VERSION 2.8)

SET(-D_GNU_SOURCE)

add_library(zmalloc ../common/zmalloc.h ../common/zmalloc.c)
add_library(adlist ../common/adlist.c ../common/adlist.h)
add_library(ae ../common/ae.h ../common/ae.c)
add_library(anet ../common/anet.h ../common/anet.c)
add_library(util ../common/util.h ../common/util.c)
add_library(sds ../common/sds.h ../common/sds.c)
add_library(fort ../common/fort.h ../common/fort.c)

ADD_SUBDIRECTORY(test)
#ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)