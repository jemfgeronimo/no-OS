add_executable(ad7866-admt4000)

# The ADMT4000 and TMC5240 setup, the EVB glue and the STM32 platform data are
# shared with the admt4000 project. This project only adds the AD7866.
set(ADMT4000_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../admt4000)

# Resolve precision-converters-library (override/submodule/cache/fetch)
include(pcl)

target_sources(ad7866-admt4000 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/ad7866_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/iio_trigger_example/iio_trigger_example.c
    ${ADMT4000_PROJECT_DIR}/src/common/common_data.c
    ${ADMT4000_PROJECT_DIR}/src/common/iio_admt_evb.c
    ${ADMT4000_PROJECT_DIR}/src/platform/${PLATFORM}/main.c
    ${ADMT4000_PROJECT_DIR}/src/platform/${PLATFORM}/parameters.c
    ${PCL_SOURCE_DIR}/common/common.c
    ${PCL_SOURCE_DIR}/board_info/board_info.c
)

target_include_directories(ad7866-admt4000 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/examples/iio_trigger_example
    ${ADMT4000_PROJECT_DIR}/src/common
    ${ADMT4000_PROJECT_DIR}/src/platform/${PLATFORM}
    ${ADMT4000_PROJECT_DIR}/src/platform
    ${PCL_SOURCE_DIR}/common
    ${PCL_SOURCE_DIR}/board_info
    ${PCL_SOURCE_DIR}/sdp_k1_sdram
)

target_compile_definitions(ad7866-admt4000 PRIVATE
    IIO_IGNORE_BUFF_OVERRUN_ERR
    TMC=1
    TMC_DEVICE=TMC5240
)

# precision-converters board_info.c uses sprintf without including
# <stdio.h>; GCC 14+ treats implicit function declarations as errors.
# Force-include stdio.h (SHELL: keeps "-include stdio.h" as two tokens).
target_compile_options(ad7866-admt4000 PRIVATE
    "$<$<COMPILE_LANGUAGE:C>:SHELL:-include stdio.h>")

target_link_libraries(ad7866-admt4000 no-os)

config_platform_sdk(ad7866-admt4000)
add_flash_target(ad7866-admt4000)
post_build_config(ad7866-admt4000)
