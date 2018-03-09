cmake_minimum_required(VERSION 3.8)
project(sysopy1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES library/blocks.c library/blocks.h client/client.c client/tests.c client/tests.h)
add_executable(sysopy1 ${SOURCE_FILES})