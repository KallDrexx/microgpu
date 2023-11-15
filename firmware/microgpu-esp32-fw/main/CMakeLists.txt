set(DATABUS_TYPE bench)

if (DATABUS_TYPE STREQUAL "test")
    set(DATABUS_SOURCE test_databus.c)
    set(DATABUS_DEFINE DATABUS_TEST)
elseif (${DATABUS_TYPE} STREQUAL "spi")
    set(DATABUS_SOURCE spi_databus.c)
    set(DATABUS_DEFINE DATABUS_SPI)
elseif (${DATABUS_TYPE} STREQUAL "bench")
    set(DATABUS_SOURCE benchmark_databus.c)
    set(DATABUS_DEFINE DATABUS_BENCHMARK)
else ()
    message(FATAL_ERROR "No valid databus type set")
endif ()

set(MGPU_COMMON_DIR_PREFIX ../)
include(../../microgpu-common/CMakeLists.txt)

set(SOURCES
        displays/i80_display.c
        ${DATABUS_SOURCE}
        main.c
        ${MICROGPU_COMMON_SOURCES}
        ../../microgpu-common/color_rgb565.c
)

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
target_compile_definitions(${COMPONENT_LIB} PUBLIC ${DATABUS_DEFINE})
