add_executable(eval-admv1355)

# This project's Vitis 2025+ BSP uses the SDT (System Device Tree) ABI, where
# Xxx_LookupConfig() is keyed by base address instead of a classic DeviceId.
# Opt in so no-os (SPI/GPIO/UART/IRQ) and this project agree on that ABI --
# otherwise lookups silently resolve to the wrong (base 0) controller.
set(NOOS_XILINX_SDT ON)

# Common sources
target_sources(eval-admv1355 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/common_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/${PLATFORM}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/${PLATFORM}/parameters.c
)

# Basic example
if(CONFIG_EVAL_ADMV1355_BASIC_EXAMPLE)
    target_sources(eval-admv1355 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/basic/basic_example.c
    )
    target_include_directories(eval-admv1355 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/basic
    )
    target_compile_definitions(eval-admv1355 PRIVATE BASIC_EXAMPLE)
endif()

# IIO example
if(CONFIG_EVAL_ADMV1355_IIO_EXAMPLE)
    target_sources(eval-admv1355 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/iio_example/iio_example.c
    )
    target_include_directories(eval-admv1355 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/iio_example
    )
    target_compile_definitions(eval-admv1355 PRIVATE IIO_EXAMPLE)
endif()

target_include_directories(eval-admv1355 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/${PLATFORM}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
)

target_link_libraries(eval-admv1355 no-os)

config_platform_sdk(eval-admv1355)
add_flash_target(eval-admv1355)
post_build_config(eval-admv1355)
