set(SOURCES
        ../../microgpu-common/framebuffer.c
        ../../microgpu-common/color_rgb565.c
        ../../microgpu-common/messages.c
        ../../microgpu-common/operation_deserializer.c
        ../../microgpu-common/operation_execution.c
        ../../microgpu-common/operations/drawing/rectangle.c
        ../../microgpu-common/operations/drawing/triangle.c
        ../../microgpu-common/operations/get_last_message.c
        ../../microgpu-common/operations/present_framebuffer.c
        ../../microgpu-common/operations/reset.c
        ../../microgpu-common/operations/status.c
        ../../microgpu-common/response_serializer.c
        displays/i80_display.c
        spi_databus.c
        main.c
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} ${ARGV0} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
