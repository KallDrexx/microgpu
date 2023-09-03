cmake_minimum_required(VERSION 3.26)
project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)
set(DATABUS_TYPE tcp)

find_package(SDL2 CONFIG REQUIRED)

include(CMakeLists.tcp.txt)
include(CMakeLists.test.txt)
include_directories(../)
