cmake_minimum_required(VERSION 3.26)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

project(microgpu_sdl_fw C)

set(CMAKE_C_STANDARD 17)
set(DATABUS_TYPE tcp)

find_package(SDL2 CONFIG REQUIRED)

include(../microgpu-common/CMakeLists.txt)

function(create_sdl_target)
    # First argument should be the databus prefix, the second should be the define name
    add_executable(microgpu_sdl_fw_${ARGV0}
            main.c
            sdl_display.c
            ${ARGV0}_databus.c
            ${MICROGPU_COMMON_SOURCES}
            ../microgpu-common/color_rgb565.c
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
