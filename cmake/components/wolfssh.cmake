
file(GLOB WOLFSSH_SRCS ${CMAKE_SOURCE_DIR}/external/wolfssh/src/*.c)
list(REMOVE_ITEM WOLFSSH_SRCS "${CMAKE_SOURCE_DIR}/external/wolfssh/src/misc.c")

add_library(wolfssh ${WOLFSSH_SRCS})

target_compile_definitions(wolfssh PUBLIC WOLFSSH_LWIP)

target_include_directories(wolfssh PUBLIC "${CMAKE_SOURCE_DIR}/external/wolfssh/")
target_include_directories(wolfssh PRIVATE "${CMAKE_SOURCE_DIR}/include/wolfssh")
target_include_directories(wolfssh PRIVATE "external/stm32h7xx_hal_driver/Inc" "include/hal")
target_include_directories(wolfssh PRIVATE "external/cmsis_core/Include" "external/cmsis_device_h7/Include")

target_link_libraries(wolfssh wolfssl lwip)