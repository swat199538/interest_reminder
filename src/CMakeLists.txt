cmake_minimum_required(VERSION 2.8)

SET(-D_GNU_SOURCE)

ADD_SUBDIRECTORY(server)

ADD_SUBDIRECTORY(test)

ADD_SUBDIRECTORY(client)