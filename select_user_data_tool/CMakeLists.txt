# Copyright (C) 2013 - XuKoo Inc.
# Author: davidluan
# CreateTime: 2013-09-02

file(GLOB_RECURSE main_srcs *.cpp)


build_exe(select_user_data_tool
    ${main_srcs}
    OUTPUT_DIR ${ROOT_DIR}/bin/
    LIBS libutil
    DEPENDS libutil
    )
