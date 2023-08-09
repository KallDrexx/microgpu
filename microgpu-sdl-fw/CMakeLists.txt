cmake_minimum_required(VERSION 3.26)
project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)

find_package(SDL2 CONFIG REQUIRED)

add_executable(microgpu_sdl_fw
        main.c
        display.c
        input.c
)

# TODO: We need to split out 565 code into a separate target
# in order to not force all targets to use 565.
target_compile_definitions(microgpu_common PUBLIC MGPU_COLOR_MODE_USE_RGB565)

target_link_libraries(microgpu_sdl_fw
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_link_libraries(microgpu_sdl_fw
        PRIVATE
        microgpu_common
)

include_directories(../microgpu-common)
