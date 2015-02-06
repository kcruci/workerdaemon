# Copyright (C) 2013 - Inc.
# Author: davidluan
# CreateTime: 2013-09-02

file(GLOB_RECURSE unittest_srcs *test.cpp)


build_unittest(unittests
    ${unittest_srcs}
    OUTPUT_DIR ${ROOT_DIR}/run/unittest
    LIBS libutil gtest gtest_main
    DEPENDS libutil
    )
