cmake_minimum_required(VERSION 3.5)
#############
project(cpx)
#############

#____________________________________________________________________________________
# define
set(HOME "/home/michel")
 set(PRJ_PARENT "${HOME}/Projects/Kdevelop")
  set(INCLUDE_PATH "${PRJ_PARENT}/util/")
set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -ggdb")

#link_directories(${PRJ_PARENT}/build/util)

add_definitions(
)

add_compile_options(
    -march=native
    -save-temps
    -std=gnu++23
    -fconcepts
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

#find_library( UTIL_PATH tosicsutil "${PRJ_PARENT}/build" )
set(UTIL_PATH "${PRJ_PARENT}/build/util/libutil.a")
message ("And the path is: ${UTIL_PATH}")

target_link_libraries(
    cpx
    ${UTIL_PATH}
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
