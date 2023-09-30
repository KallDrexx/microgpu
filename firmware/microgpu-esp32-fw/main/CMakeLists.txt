set(DATABUS_TYPE spi)

if (DATABUS_TYPE STREQUAL "test")
    set(DATABUS_SOURCE test_databus.c)
    set(DATABUS_DEFINE DATABUS_TEST)
elseif (${DATABUS_TYPE} STREQUAL "spi")
    set(DATABUS_SOURCE spi_databus.c)
    set(DATABUS_DEFINE DATABUS_SPI)
else ()
    message(FATAL_ERROR "No valid databus type set")
endif ()

set(SOURCES
        ../../microgpu-common/framebuffer.c
        ../../microgpu-common/color_rgb565.c
        ../../microgpu-common/messages.c
        ../../microgpu-common/operation_deserializer.c
        ../../microgpu-common/operation_execution.c
        ../../microgpu-common/operations/batch.c
        ../../microgpu-common/operations/drawing/rectangle.c
        ../../microgpu-common/operations/drawing/triangle.c
        ../../microgpu-common/operations/get_last_message.c
        ../../microgpu-common/operations/present_framebuffer.c
        ../../microgpu-common/operations/reset.c
        ../../microgpu-common/operations/status.c
        ../../microgpu-common/response_serializer.c
        displays/i80_display.c
        ${DATABUS_SOURCE}
        main.c
)

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
target_compile_definitions(${COMPONENT_LIB} PUBLIC ${DATABUS_DEFINE})
