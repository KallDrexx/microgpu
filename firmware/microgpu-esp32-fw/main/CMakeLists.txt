set(SOURCES main.c
        displays/i80_display.c
        ../../microgpu-common/framebuffer.c
        ../../microgpu-common/color_rgb565.c
        ../../microgpu-common/operations/drawing/rectangle.c
        ../../microgpu-common/operations/drawing/triangle.c
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} ${ARGV0} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
