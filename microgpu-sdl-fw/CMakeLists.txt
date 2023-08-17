cmake_minimum_required(VERSION 3.26)
project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)

find_package(SDL2 CONFIG REQUIRED)

add_executable(microgpu_sdl_fw
        main.c
        sdl_display.c
        input.c
        ../microgpu-common/framebuffer.c
        ../microgpu-common/color_rgb565.c
        ../microgpu-common/operations.c
        ../microgpu-common/drawing/rectangle.c
        ../microgpu-common/drawing/rectangle.h
        ../microgpu-common/drawing/triangle.c
        ../microgpu-common/drawing/triangle.h
)

target_compile_definitions(microgpu_sdl_fw PUBLIC MGPU_COLOR_MODE_USE_RGB565)

target_link_libraries(microgpu_sdl_fw
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

include_directories(../)
