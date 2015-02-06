# Copyright (C) 2013 - XuKoo Inc.
# Author: davidluan
# CreateTime: 2013-09-02

file(GLOB_RECURSE cli_srcs *.cpp)


build_exe(cli_exe
    ${cli_srcs}
    OUTPUT_DIR ${ROOT_DIR}/clitest/
    LIBS libutil
    DEPENDS libutil
    )
