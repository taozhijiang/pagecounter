cmake_minimum_required (VERSION 2.8.11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x " )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -Werror=format -march=native " )
include("${PROJECT_SOURCE_DIR}/help.cmake")
define_colors()

####################
### 修改编译选项 ###

# if want to see debug in verbose mode
# cmake -DCMAKE_VERBOSE_MAKEFILE=true ../
# if want to see build in detail
# make -- VERBOSE=1

option(BUILD_DEBUG "Build Project for Debug Propurse..." OFF)

if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "${Red}build_type Debug flag: ${CMAKE_CXX_FLAGS_DEBUG}${ColourReset}")
else(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    #add_definitions(-DNP_DEBUG)
    message(STATUS "${Red}build_type RelWithDebInfo flag: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}${ColourReset}")
endif(BUILD_DEBUG)

if(EXISTS "${PROJECT_SOURCE_DIR}/VERSION")
    file(READ "${PROJECT_SOURCE_DIR}/VERSION" PROGRAM_VERSION)
    string(STRIP "${PROGRAM_VERSION}" PROGRAM_VERSION)
endif()

configure_file (
    "include/version.h.in"
    "../include/version.h"
    @ONLY )

###################


set (INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/source/
    ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/include/
    ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/include/google/
    ${PROJECT_SOURCE_DIR}/deps/roo
    ${PROJECT_SOURCE_DIR}/deps/
)
include_directories( ${INCLUDE_DIRS} )


add_subdirectory( ${PROJECT_SOURCE_DIR}/deps/roo )
add_subdirectory( ${PROJECT_SOURCE_DIR}/deps/tzhttpd)

link_directories(
        /usr/lib64/mysql/
        ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/libs/
        ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/libs/boost/
        ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/libs/google/
        ${PROJECT_SOURCE_DIR}/deps/xtra_rhelz.x/libs/google/protobuf-2.5.0/
)

# local temp generated file
exec_program( "export BUILD_VAR=`git log -1 --pretty=%H` && echo 'const char *build_commit_version = \"VCS: Commit:' $BUILD_VAR '\";' > build_version.cpp ")
exec_program( "export BUILD_VAR=`git symbolic-ref HEAD` && echo 'const char *build_commit_branch = \"VCS: Branch:' $BUILD_VAR '\";' >> build_version.cpp ")
exec_program( "export BUILD_VAR=`git log -1 --pretty=%cd` && echo 'const char *build_commit_date = \"VCS: Date:' $BUILD_VAR '\";' >> build_version.cpp ")
exec_program( "export BUILD_VAR=`git log -1 --pretty=\"%an %ae\"` && echo 'const char *build_commit_author = \"VCS: Author:' $BUILD_VAR '\";' >> build_version.cpp ")
exec_program( "export BUILD_VAR=`date +%F` && echo 'const char *build_time = \"Build At:' $BUILD_VAR '\";' >> build_version.cpp ")

aux_source_directory(source PROJECT_SRCS)
add_executable( page_counter_service main.cpp setup.cpp build_version.cpp ${PROJECT_SRCS})

# ld iconv ?

set (EXTRA_LIBS ${EXTRA_LIBS} tzhttpd Roo )

set (EXTRA_LIBS ${EXTRA_LIBS} cryptopp crypto ssl config++)
set (EXTRA_LIBS ${EXTRA_LIBS} mysqlcppconn jsoncpp)
set (EXTRA_LIBS ${EXTRA_LIBS} pthread)
set (EXTRA_LIBS ${EXTRA_LIBS} boost_system boost_thread boost_chrono boost_regex )
set (EXTRA_LIBS ${EXTRA_LIBS} protoc protobuf )
set (EXTRA_LIBS ${EXTRA_LIBS} glog_syslog )



# 某些库需要静态全连接，否则主服务没使用但是so使用，会报符号没有找到
target_link_libraries(
    page_counter_service -lrt -ldl
    ${EXTRA_LIBS}
)

if(BUILD_DEBUG)
    message(STATUS "${Red}we will build examples and unit_tests ...${ColourReset}")
    
    find_program(MEMORYCHECK_COMMAND NAMES valgrind)
    if(MEMORYCHECK_COMMAND)
        # ctest -T memcheck
        set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
        include(CTest)
    endif()

    enable_testing()
    add_subdirectory( test )
endif(BUILD_DEBUG)
