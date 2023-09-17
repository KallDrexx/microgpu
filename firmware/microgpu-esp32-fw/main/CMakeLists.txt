idf_component_register(SRCS "main.c" "displays/i80_display.c"
                    INCLUDE_DIRS "." "../../")

target_compile_definitions(${COMPONENT_LIB} ${ARGV0} PUBLIC MGPU_COLOR_MODE_USE_RGB565)
