cmake_minimum_required(VERSION 3.26)
project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)

find_package(SDL2 CONFIG REQUIRED)

add_executable(microgpu_sdl_fw
        main.c
        basic_databus.c
        sdl_display.c
        ../microgpu-common/alloc.h
        ../microgpu-common/color_rgb565.c
        ../microgpu-common/framebuffer.c
        ../microgpu-common/messages.c
        ../microgpu-common/operation_deserializer.c
        ../microgpu-common/operation_execution.c
        ../microgpu-common/operations/drawing/rectangle.c
        ../microgpu-common/operations/drawing/triangle.c
        ../microgpu-common/operations/get_last_message.c
        ../microgpu-common/operations/present_framebuffer.c
        ../microgpu-common/operations/reset.c
        ../microgpu-common/operations/status.c
        tcp_databus.c
        tcp_databus.h
)

target_compile_definitions(microgpu_sdl_fw PUBLIC MGPU_COLOR_MODE_USE_RGB565)

target_link_libraries(microgpu_sdl_fw
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

include_directories(../)
