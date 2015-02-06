#########################################################################GG
# Copyright (C) 2013
# Author: davidluan
# CreateTime: 2013-08-02
#
# 1) custom macro to build *c *cpp => ../bin/*.exe
# BUILD_EXE(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ../bin/
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
#
# 2) custom macro to build *c *cpp => ./*.exe
# BUILD_TEST(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ./
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]               # internal automatically add gtest gtest_main
#   [DEPENDS depend1 depend2 ...]
#   )
#   internal automatically add_test()
#
# 3) custom macro to build *c *cpp => ../lib/lib*.a
# BUILD_STATIC_LIB(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ../lib/
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
#
# 4) custom macro to build *.proto => *.h *.cc
# BUILD_PB2HCC(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ./
#   )
#
# 5) custom macro to build *unittest.cpp => *unitetest.exe
# BUILD_UNITTEST(targetname src1 src2 ...
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
#
#########################################################################

if(__XUKOO_BUILD_MACRO_INCLUDED)
    return()
endif()
set(__XUKOO_BUILD_MACRO_INCLUDED TRUE)

cmake_minimum_required(VERSION 2.8)

include(CMakeParseArguments)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
#set(CUSTOM_MACRO_VERBOSE TRUE)

set(PROTOBUF_TOOL /usr/bin/protoc CACHE string "")

message(STATUS "\${PROTOBUF_TOOL}: ${PROTOBUF_TOOL}")

macro(CAR var)
    set(${var} ${ARGV1})
endmacro()

macro(CDR var junk)
    set(${var} ${ARGN})
endmacro()

macro(LIST_LENGTH var)
    set(entries)
    foreach(e ${ARGN})
        set(entries "${entries}.")
    endforeach(e)
    string(LENGTH ${entries} ${var})
endmacro(LIST_LENGTH)

macro(LIST_INDEX var index)
    set(list . ${ARGN})
    foreach(i RANGE 1 ${index})
        cdr(list ${list})
    endforeach(i)
    car(${var} ${list})
endmacro(LIST_INDEX)

# custom macro to build *c *cpp => ../bin/*.exe
# BUILD_EXE(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ../bin/
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
macro(BUILD_EXE)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR;OUTPUT_NAME"
        "DEFINES;INCLUDE_DIRS;LIB_DIRS;LIBS;DEPENDS"
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})
    message(STATUS "BUILD_EXE(${BUILD_TARGET})")
    if(NOT BUILD_SRC_LIST)
        message(FATAL_ERROR "\${SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${SRC_LIST}: ${BUILD_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
    endif()
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()
    if(NOT BUILD_OUTPUT_NAME)
        set(BUILD_OUTPUT_NAME ${BUILD_TARGET})
    endif()
    get_filename_component(BUILD_OUTPUT_NAME ${BUILD_OUTPUT_NAME} NAME)
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_NAME}: ${BUILD_OUTPUT_NAME}")
    endif()
    project(${BUILD_TARGET})
    if(BUILD_LIB_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIB_DIRS}: ${BUILD_LIB_DIRS}")
        endif()
        link_directories(${BUILD_LIB_DIRS})
    endif()
    add_executable(${BUILD_TARGET} ${BUILD_SRC_LIST})
    set_target_properties(${BUILD_TARGET} PROPERTIES RUNTIME_OUTPUT_NAME "${BUILD_OUTPUT_NAME}")
    set_target_properties(${BUILD_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}")
    if(BUILD_DEFINES)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEFINES}: ${BUILD_DEFINES}")
        endif()
        target_compile_definitions(${BUILD_TARGET} PUBLIC ${BUILD_DEFINES})
    endif()
    if(BUILD_INCLUDE_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${INCLUDE_DIRS}: ${BUILD_INCLUDE_DIRS}")
        endif()
        target_include_directories(${BUILD_TARGET} PUBLIC ${BUILD_INCLUDE_DIRS})
    endif()
    if(BUILD_LIBS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIBS}: ${BUILD_LIBS}")
        endif()
        target_link_libraries(${BUILD_TARGET} ${BUILD_LIBS})
    endif()
    if(BUILD_DEPENDS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEPENDS}: ${BUILD_DEPENDS}")
        endif()
        add_dependencies(${BUILD_TARGET} ${BUILD_DEPENDS})
    endif()
endmacro()

# custom macro to build *c *cpp => ../lib/lib*.a
# BUILD_STATIC_LIB(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ../lib/
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
macro(BUILD_STATIC_LIB)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR;OUTPUT_NAME"
        "DEFINES;INCLUDE_DIRS;LIB_DIRS;LIBS;DEPENDS"
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})
    message(STATUS "BUILD_STATIC_LIB(${BUILD_TARGET})")
    if(NOT BUILD_SRC_LIST)
        message(FATAL_ERROR "\${SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${SRC_LIST}: ${BUILD_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
    endif()
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()
    if(NOT BUILD_OUTPUT_NAME)
        set(BUILD_OUTPUT_NAME ${BUILD_TARGET})
    endif()
    get_filename_component(BUILD_OUTPUT_NAME ${BUILD_OUTPUT_NAME} NAME)
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_NAME}: ${BUILD_OUTPUT_NAME}")
    endif()
    project(${BUILD_TARGET})
    if(BUILD_LIB_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIB_DIRS}: ${BUILD_LIB_DIRS}")
        endif()
        link_directories(${BUILD_LIB_DIRS})
    endif()
    add_library(${BUILD_TARGET} STATIC ${BUILD_SRC_LIST})
    set_target_properties(${BUILD_TARGET} PROPERTIES ARCHIVE_OUTPUT_NAME "${BUILD_OUTPUT_NAME}")
    set_target_properties(${BUILD_TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}")
    if(BUILD_DEFINES)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEFINES}: ${BUILD_DEFINES}")
        endif()
        target_compile_definitions(${BUILD_TARGET} PUBLIC ${BUILD_DEFINES})
    endif()
    if(BUILD_INCLUDE_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${INCLUDE_DIRS}: ${BUILD_INCLUDE_DIRS}")
        endif()
        target_include_directories(${BUILD_TARGET} PUBLIC ${BUILD_INCLUDE_DIRS})
    endif()
    if(BUILD_LIBS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIBS}: ${BUILD_LIBS}")
        endif()
        target_link_libraries(${BUILD_TARGET} ${BUILD_LIBS})
    endif()
    if(BUILD_DEPENDS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEPENDS}: ${BUILD_DEPENDS}")
        endif()
        add_dependencies(${BUILD_TARGET} ${BUILD_DEPENDS})
    endif()
endmacro()

# custom macro to build *c *cpp => ../lib/lib*.so
# BUILD_SHARED_LIB(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ../lib/
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]
#   [DEPENDS depend1 depend2 ...]
#   )
macro(BUILD_SHARED_LIB)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR;OUTPUT_NAME"
        "DEFINES;INCLUDE_DIRS;LIB_DIRS;LIBS;DEPENDS"
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})
    message(STATUS "BUILD_SHARED_LIB(${BUILD_TARGET})")
    if(NOT BUILD_SRC_LIST)
        message(FATAL_ERROR "\${SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${SRC_LIST}: ${BUILD_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
    endif()
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()
    if(NOT BUILD_OUTPUT_NAME)
        set(BUILD_OUTPUT_NAME ${BUILD_TARGET})
    endif()
    get_filename_component(BUILD_OUTPUT_NAME ${BUILD_OUTPUT_NAME} NAME)
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_NAME}: ${BUILD_OUTPUT_NAME}")
    endif()
    project(${BUILD_TARGET})
    if(BUILD_LIB_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIB_DIRS}: ${BUILD_LIB_DIRS}")
        endif()
        link_directories(${BUILD_LIB_DIRS})
    endif()
    add_library(${BUILD_TARGET} SHARED ${BUILD_SRC_LIST})
    set_target_properties(${BUILD_TARGET} PROPERTIES LIBRARY_OUTPUT_NAME "${BUILD_OUTPUT_NAME}")
    set_target_properties(${BUILD_TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}")
    if(BUILD_DEFINES)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEFINES}: ${BUILD_DEFINES}")
        endif()
        target_compile_definitions(${BUILD_TARGET} PUBLIC ${BUILD_DEFINES})
    endif()
    if(BUILD_INCLUDE_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${INCLUDE_DIRS}: ${BUILD_INCLUDE_DIRS}")
        endif()
        target_include_directories(${BUILD_TARGET} PUBLIC ${BUILD_INCLUDE_DIRS})
    endif()
    if(BUILD_LIBS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIBS}: ${BUILD_LIBS}")
        endif()
        target_link_libraries(${BUILD_TARGET} ${BUILD_LIBS})
    endif()
    if(BUILD_DEPENDS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEPENDS}: ${BUILD_DEPENDS}")
        endif()
        add_dependencies(${BUILD_TARGET} ${BUILD_DEPENDS})
    endif()
endmacro()

# custom macro to build *c *cpp => ./*.exe
# BUILD_TEST(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ./
#   [OUTPUT_NAME output_name]           # default ${targetname}
#   [DEFINES define1 define2 ...]
#   [INCLUDE_DIRS include_dir1 include_dir2 ...]
#   [LIB_DIRS lib_dir1 lib_dir2 ...]
#   [LIBS lib1 lib2 ... ]               # internal automatically add gtest gtest_main
#   [DEPENDS depend1 depend2 ...]
#   )
macro(BUILD_TEST)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR;OUTPUT_NAME"
        "DEFINES;INCLUDE_DIRS;LIB_DIRS;LIBS;DEPENDS"
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})
    message(STATUS "BUILD_EXE(${BUILD_TARGET})")
    if(NOT BUILD_SRC_LIST)
        message(FATAL_ERROR "\${SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${SRC_LIST}: ${BUILD_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
    endif()
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()
    if(NOT BUILD_OUTPUT_NAME)
        set(BUILD_OUTPUT_NAME ${BUILD_TARGET})
    endif()
    get_filename_component(BUILD_OUTPUT_NAME ${BUILD_OUTPUT_NAME} NAME)
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_NAME}: ${BUILD_OUTPUT_NAME}")
    endif()
    project(${BUILD_TARGET})
    if(BUILD_LIB_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIB_DIRS}: ${BUILD_LIB_DIRS}")
        endif()
        link_directories(${BUILD_LIB_DIRS})
    endif()
    add_executable(${BUILD_TARGET} ${BUILD_SRC_LIST})
    set_target_properties(${BUILD_TARGET} PROPERTIES RUNTIME_OUTPUT_NAME "${BUILD_OUTPUT_NAME}")
    set_target_properties(${BUILD_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}")
    if(BUILD_DEFINES)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEFINES}: ${BUILD_DEFINES}")
        endif()
        target_compile_definitions(${BUILD_TARGET} PUBLIC ${BUILD_DEFINES})
    endif()
    if(BUILD_INCLUDE_DIRS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${INCLUDE_DIRS}: ${BUILD_INCLUDE_DIRS}")
        endif()
        target_include_directories(${BUILD_TARGET} PUBLIC ${BUILD_INCLUDE_DIRS})
    endif()
    target_link_libraries(${BUILD_TARGET} gtest gtest_main pthread)
    if(BUILD_LIBS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${LIBS}: ${BUILD_LIBS}")
        endif()
        target_link_libraries(${BUILD_TARGET} ${BUILD_LIBS})
    endif()
    target_link_libraries(${BUILD_TARGET} gtest gtest_main)
    if(BUILD_DEPENDS)
        if(CUSTOM_MACRO_VERBOSE)
            message(STATUS "\${DEPENDS}: ${BUILD_DEPENDS}")
        endif()
        add_dependencies(${BUILD_TARGET} ${BUILD_DEPENDS})
    endif()
    add_test(NAME ${BUILD_TARGET}
        WORKING_DIRECTORY ${BUILD_OUTPUT_DIR}
        COMMAND ${BUILD_OUTPUT_NAME}
        )
endmacro()

# custom macro to build *.proto => *.h *.cc
# BUILD_PB2HCC(targetname src1 src2 ...
#   [OUTPUT_DIR output_dir]             # default ./
#   )
#   )
macro(BUILD_PB2HCC)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR"
        ""
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_PROTO_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})
    message(STATUS "BUILD_PB2HCC(${BUILD_TARGET})")
    if(NOT BUILD_PROTO_SRC_LIST)
        message(FATAL_ERROR "\${PROTO_SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${PROTO_SRC_LIST}: ${BUILD_JCE_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()
    set(hfiles)
    foreach(pb ${BUILD_PROTO_SRC_LIST})
        if(NOT EXISTS ${pb})
            message(FATAL_ERROR "\${PB_SRC}: ${pb} NOT EXISTS")
        endif()
        get_filename_component(basename ${pb} NAME_WE)
        get_filename_component(pathname ${pb} PATH)
        set(hfile ${BUILD_OUTPUT_DIR}/${basename}.pb.h)
        add_custom_command(
            OUTPUT ${hfile}
            COMMAND ${PROTOBUF_TOOL} -I="${pathname}" --cpp_out=${BUILD_OUTPUT_DIR} ${pb}
            DEPENDS ${pb}
            COMMENT "Building ${hfile}"
            )
        set(hfiles ${hfiles} ${hfile})
    endforeach()
    add_custom_target(${BUILD_TARGET} DEPENDS ${hfiles})
endmacro()

macro(BUILD_UNITTEST)
    cmake_parse_arguments(BUILD
        ""
        "OUTPUT_DIR"
        "DEFINES;INCLUDE_DIRS;LIB_DIRS;LIBS;DEPENDS"
        ${ARGN}
        )
    car(BUILD_TARGET ${BUILD_UNPARSED_ARGUMENTS})
    cdr(BUILD_TEST_SRC_LIST ${BUILD_UNPARSED_ARGUMENTS})

    if(NOT BUILD_TEST_SRC_LIST)
        message(FATAL_ERROR "\${SRC_LIST} NOTFOUND")
    elseif(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${SRC_LIST}: ${BUILD_TEST_SRC_LIST}")
    endif()
    if(NOT BUILD_OUTPUT_DIR)
        set(BUILD_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    endif()
    if(CUSTOM_MACRO_VERBOSE)
        message(STATUS "\${OUTPUT_DIR}: ${BUILD_OUTPUT_DIR}")
    endif()

    foreach(srcfile ${BUILD_TEST_SRC_LIST})
        if(NOT EXISTS ${srcfile})
            message(FATAL_ERROR "\${UNITTEST_SRC}: ${srcfile} NOT EXISTS")
        endif()
        get_filename_component(testname ${srcfile} NAME_WE)
        message(STATUS "BUILD_UNITTEST(${testname})")
        add_executable(${testname} ${srcfile})
        set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_NAME  "${testname}")
        set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}")
        if(BUILD_DEFINES)
            if(CUSTOM_MACRO_VERBOSE)
                message(STATUS "\${DEFINES}: ${BUILD_DEFINES}")
            endif()
            target_compile_definitions(${testname} PUBLIC ${BUILD_DEFINES})
        endif()
        if(BUILD_INCLUDE_DIRS)
            if(CUSTOM_MACRO_VERBOSE)
                message(STATUS "\${INCLUDE_DIRS}: ${BUILD_INCLUDE_DIRS}")
            endif()
            target_include_directories(${testname} PUBLIC ${BUILD_INCLUDE_DIRS})
        endif()
        if(BUILD_LIBS)
            if(CUSTOM_MACRO_VERBOSE)
                message(STATUS "\${LIBS}: ${BUILD_LIBS}")
            endif()
            target_link_libraries(${testname} ${BUILD_LIBS})
        endif()
        if(BUILD_DEPENDS)
            if(CUSTOM_MACRO_VERBOSE)
                message(STATUS "\${DEPENDS}: ${BUILD_DEPENDS}")
            endif()
            add_dependencies(${testname} ${BUILD_DEPENDS})
        endif()


        if(BUILD_LIB_DIRS)
            if(CUSTOM_MACRO_VERBOSE)
                message(STATUS "\${LIB_DIRS}: ${BUILD_LIB_DIRS}")
            endif()
            link_directories(${BUILD_LIB_DIRS})
        endif()
        add_test(NAME ${testname}
            WORKING_DIRECTORY ${BUILD_OUTPUT_DIR}
            COMMAND ${testname}
            )

    endforeach()

endmacro()


