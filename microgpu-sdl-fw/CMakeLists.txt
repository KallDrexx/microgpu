cmake_minimum_required(VERSION 3.26)
project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)

find_package(SDL2 CONFIG REQUIRED)
add_executable(microgpu_sdl_fw
        main.c
        display.c
        input.c
        input.h
)

target_link_libraries(microgpu_sdl_fw
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
