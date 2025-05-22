
set(WOLFSSL_FILESYSTEM "no")
set(WOLFSSL_CRYPT_TESTS "yes")
set(WOLFSSL_CRYPT_TESTS_LIBS "yes")
set(WOLFSSL_USER_SETTINGS "yes")
set(WOLFSSL_FAST_MATH "yes")
set(WOLFSSL_SINGLE_THREADED "yes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SOURCE_DIR}/include/wolfssl")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -D__ASSEMBLY__")

add_subdirectory("${CMAKE_SOURCE_DIR}/external/wolfssl")

# target_link_libraries(wolfssl PRIVATE hal cmsis)
target_sources(wolfssl PRIVATE "${CMAKE_SOURCE_DIR}/external/wolfssl/wolfcrypt/src/port/st/stm32.c"
    "${CMAKE_SOURCE_DIR}/external/wolfssl/wolfcrypt/src/port/st/stsafe.c")
target_include_directories(wolfssl PRIVATE "${CMAKE_SOURCE_DIR}/include/wolfssl")
target_include_directories(wolfssl PRIVATE "external/stm32h7xx_hal_driver/Inc" "include/hal")
target_include_directories(wolfssl PRIVATE "external/cmsis_core/Include" "external/cmsis_device_h7/Include")

target_include_directories(wolfssl PRIVATE 
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/include"
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/CMSIS_RTOS_V2"
    "${CMAKE_SOURCE_DIR}/external/stm32_mw_freertos/Source/portable/${FREERTOS_PORT}"
    "${CMAKE_SOURCE_DIR}/include/rtos"
)
target_include_directories(wolfssl PRIVATE 
    "external/stm32_mw_lwip/src/include"
    "external/stm32_mw_lwip/system"
    "include/lwip"
    )

target_include_directories(wolfcrypttest_lib PRIVATE "${CMAKE_SOURCE_DIR}/include/wolfssl")
target_include_directories(wolfcrypttest_lib PRIVATE "external/stm32h7xx_hal_driver/Inc" "include/hal")
target_include_directories(wolfcrypttest_lib PRIVATE "external/cmsis_core/Include" "external/cmsis_device_h7/Include")

target_include_directories(wolfcryptbench_lib PRIVATE "${CMAKE_SOURCE_DIR}/include/wolfssl")
target_include_directories(wolfcryptbench_lib PRIVATE "external/stm32h7xx_hal_driver/Inc" "include/hal")
target_include_directories(wolfcryptbench_lib PRIVATE "external/cmsis_core/Include" "external/cmsis_device_h7/Include")

set_target_properties(wolfcrypttest wolfcryptbench PROPERTIES EXCLUDE_FROM_ALL 1 )
