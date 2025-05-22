
set(FREERTOS_PORT "GCC/ARM_CM4F")
set(FREERTOS_HEAP "heap_4.c")

# add_definitions("-DCMSIS_device_header=<stm32h7xx.h>")

file(GLOB CMSIS_RTOS_SRCS 
    ${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/*.c
    ${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/portable/${FREERTOS_PORT}/*.c
    ${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/CMSIS_RTOS_V2/cmsis_os2.c
    ${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/portable/MemMang/${FREERTOS_HEAP}
    )

add_library(cmsis-rtos ${CMSIS_RTOS_SRCS})
target_include_directories(cmsis-rtos PUBLIC 
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/include"
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/CMSIS_RTOS_V2"
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/portable/${FREERTOS_PORT}"
    "${CMAKE_SOURCE_DIR}/include/rtos"
)
target_link_libraries(cmsis-rtos hal cmsis)