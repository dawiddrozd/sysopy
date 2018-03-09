cmake_minimum_required(VERSION 3.8)
project(sysopy1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES library/lib.c library/lib.h library/client.c library/tests.c library/tests.h)
add_executable(sysopy1 ${SOURCE_FILES})