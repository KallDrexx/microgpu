set(MGPU_COMMON_DIR_PREFIX ../)
include(../../microgpu-common/CMakeLists.txt)

set(SOURCES
        displays/i80_display.c
        main.c
        ${MICROGPU_COMMON_SOURCES}
        ../../microgpu-common/colors/color_rgb565.c
)

if (CONFIG_MICROGPU_DATABUS_SPI)
    list(APPEND SOURCES "spi_databus.c")
elseif (CONFIG_MICROGPU_DATABUS_BENCHMARK)
    list(APPEND SOURCES "benchmark_databus.c")
endif ()

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
target_compile_definitions(${COMPONENT_LIB} PUBLIC ${DATABUS_DEFINE})
target_compile_definitions(${COMPONENT_LIB} PUBLIC MGPU_VERSION="${PROJECT_VER}")
