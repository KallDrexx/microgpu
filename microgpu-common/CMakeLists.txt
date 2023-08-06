cmake_minimum_required(VERSION 3.26)
project(microgpu_common)

set(CMAKE_C_STANDARD 17)
add_library(microgpu_common
        color.c
)