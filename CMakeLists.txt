cmake_minimum_required(VERSION 3.26)

project(microgpu)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_subdirectory(microgpu-common)
add_subdirectory(microgpu-sdl-fw)
