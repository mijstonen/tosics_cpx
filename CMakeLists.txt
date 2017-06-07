cmake_minimum_required(VERSION 3.5)
#############
project(cpx)
#############

#____________________________________________________________________________________
# define

set(PRJ_PARENT "/home/michel/Projects/Kdevelop")
 set(INCLUDE_PATH "${PRJ_PARENT}/util/")
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -ggdb")

link_directories(${PRJ_PARENT}/build/util)

add_definitions(
)

add_compile_options(
    -march=native 
    -save-temps 
    -std=c++14 
    -ggdb
    -I${INCLUDE_PATH}
    -include stdinc.hpp 
    -Wall -Wextra -Wold-style-cast 
    -x c++
    )
#____________________________________________________________________________________
# compile

add_executable(
    cpx 
    main.cpp 
    runner2.cpp
)

target_link_libraries(
    cpx
    util
    stdc++fs
    pthread
    dl
)

#____________________________________________________________________________________
# test
install(TARGETS cpx RUNTIME DESTINATION bin)

#____________________________________________________________________________________
# install
# add_subdirectory(deps)  nested CMakeList.txt
