cmake_minimum_required(VERSION 3.26)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)
set(DATABUS_TYPE tcp)

find_package(SDL2 CONFIG REQUIRED)

function(create_sdl_target)
    # First argument should be the databus prefix, the second should be the define name
    add_executable(microgpu_sdl_fw_${ARGV0}
            main.c
            sdl_display.c
            ${ARGV0}_databus.c
            ../microgpu-common/alloc.h
            ../microgpu-common/color_rgb565.c
            ../microgpu-common/framebuffer.c
            ../microgpu-common/messages.c
            ../microgpu-common/operation_deserializer.c
            ../microgpu-common/operation_execution.c
            ../microgpu-common/operations/batch.c
            ../microgpu-common/operations/drawing/rectangle.c
            ../microgpu-common/operations/drawing/triangle.c
            ../microgpu-common/operations/get_last_message.c
            ../microgpu-common/operations/present_framebuffer.c
            ../microgpu-common/operations/reset.c
            ../microgpu-common/operations/status.c
            ../microgpu-common/response_serializer.c
    )

    target_compile_definitions(microgpu_sdl_fw_${ARGV0} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
    target_compile_definitions(microgpu_sdl_fw_${ARGV0} PUBLIC ${ARGV1})

    target_link_libraries(microgpu_sdl_fw_${ARGV0}
            PRIVATE
            $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
            $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )
endfunction()

create_sdl_target(tcp DATABUS_TCP)
create_sdl_target(test DATABUS_BASIC)
include_directories(../)
