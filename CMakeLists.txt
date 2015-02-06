#########################################################################
# Copyright (C) 2014 .
# Author: davidluan
# CreateTime: 2014-02-02
#########################################################################
cmake_minimum_required(VERSION 2.8)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)


# source root dir
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "" )

message(STATUS "\${ROOT_DIR}: "${ROOT_DIR})

include(${ROOT_DIR}/cmake/comm_build_macro.cmake)

# install dir
# TODO: cmake -DCMAKE_INSTALL_PREFIX=/usr
#set(CMAKE_INSTALL_PREFIX "${ROOT_DIR}/release/xxxproject2")

# debug
set(CMAKE_BUILD_TYPE DEBUG)


# gcc warning settings
# see@ -Wshadow
# http://stackoverflow.com/questions/1339397/more-fine-grained-behavior-of-gcc-wshadow-option
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE OGRE_GCC_VERSION)
    message("${CMAKE_CXX_COMPILER}(need 4.4 or higher) NOW: ${OGRE_GCC_VERSION} ")

    add_definitions(
        -Wall -Wextra -Wformat=2
        -Wno-unused-parameter -Wmissing-field-initializers
        -Wlong-long #-Wshadow
        -Wmissing-include-dirs
        -Wpointer-arith
        -fpermissive
        )
endif ()

add_definitions(-D__STDC_FORMAT_MACROS -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_DEBUG)


# TODO svn revision
# svn_get_revision(${ROOT_DIR} SVN_REVISION)
# add_definitions(-DSVN_REVISION=${SVN_REVISION})

# common include dirs
include_directories(
    ${ROOT_DIR}
    ${ROOT_DIR}/3rdparty
    ${ROOT_DIR}/3rdparty/curl/include
    ${ROOT_DIR}/3rdparty/gtest/include
    ${ROOT_DIR}/3rdparty/jsoncpp/include
    ${ROOT_DIR}/3rdparty/oauth/include
    ${ROOT_DIR}/3rdparty/openssl/include
    ${ROOT_DIR}/3rdparty/tinyxml/include
    ${ROOT_DIR}/3rdparty/hiredis/include
    ${ROOT_DIR}/3rdparty/log4cpp/include
    ${ROOT_DIR}/3rdparty/libmemcached/include
    ${ROOT_DIR}/3rdparty/mysql/include
    ${ROOT_DIR}/3rdparty/fcgi/include
    ${ROOT_DIR}/3rdparty/libqos_client64-3.2.0
    ${ROOT_DIR}/3rdparty/tencent_sdk_lib/include
    ${ROOT_DIR}/util
)

# common lib dirs
link_directories(
    ${ROOT_DIR}
    ${ROOT_DIR}/3rdparty/curl/lib
    ${ROOT_DIR}/3rdparty/gtest/lib
    ${ROOT_DIR}/3rdparty/jsoncpp/lib
    ${ROOT_DIR}/3rdparty/oauth/lib
    ${ROOT_DIR}/3rdparty/openssl/lib
    ${ROOT_DIR}/3rdparty/tinyxml/lib
    ${ROOT_DIR}/3rdparty/google/lib
    ${ROOT_DIR}/3rdparty/hiredis/lib
    ${ROOT_DIR}/3rdparty/log4cpp/lib
    ${ROOT_DIR}/3rdparty/libmemcached/lib
    ${ROOT_DIR}/3rdparty/mysql/lib
    ${ROOT_DIR}/3rdparty/fcgi/lib
    ${ROOT_DIR}/3rdparty/tencent_sdk_lib/lib
    ${ROOT_DIR}/3rdparty/libqos_client64-3.2.0
)

# default output bin path ../bin
# default output library path ../lib

# enable testing
enable_testing()

# sub-project directories
add_subdirectory(util)
add_subdirectory(unittest)
add_subdirectory(daemon)
add_subdirectory(clitest)
add_subdirectory(select_user_data_tool)
add_subdirectory(recache)
add_subdirectory(trans_gem_svr)
add_subdirectory(t_pay_callback)
