cmake_minimum_required(VERSION 3.25)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/arm-none-eabi-gcc.cmake")
endif()

set(CMAKE_BUILD_TYPE "DEBUG")
set(BUILD_SHARED_LIBS off)


project(tty2eth C ASM)


set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32H735IGKx_FLASH.ld)
set(MCU "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
set(COMMON_FLAGS "${MCU} -Og -Wall -fdata-sections -ffunction-sections -Wno-error=sign-compare")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -MMD -MP")
set(CMAKE_EXE_LINKER_FLAGS "${MCU}  -u _printf_float -T ${LINKER_SCRIPT} -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--undefined=uxTopUsedPriority -Wl,--gc-sections -Wl,--print-memory-usage -fno-pthread")

add_definitions(
    -DLWIP_DEBUG
    -DWOLFSSH_SSHD
    -DDEBUG_WOLFSSH
    -DUSE_HAL_DRIVER
    -DUSE_FULL_LL_DRIVER
    -DSTM32H735xx
    -DNO_TIMEVAL
    -DWOLFSSL_USER_SETTINGS
    -DWOLFSSL_USER_SETTINGS_ASM
    -DDATA_IN_D2_SRAM )

file(GLOB_RECURSE components ${CMAKE_SOURCE_DIR}/cmake/components/*.cmake)
foreach(component ${components})
    include(${component})
endforeach(component)


file(GLOB_RECURSE SRCS 
    ${CMAKE_SOURCE_DIR}/src/tty2eth/*.c
    ${CMAKE_SOURCE_DIR}/src/lwip/*.c
    )

set_source_files_properties(startup_stm32h735xx.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
set_property(SOURCE startup_stm32h735xx.s PROPERTY LANGUAGE C)


add_executable(${PROJECT_NAME}.elf ${SRCS} ${LINKER_SCRIPT} "startup_stm32h735xx.s")

target_include_directories(${PROJECT_NAME}.elf PUBLIC 
    "include/tty2eth"
    )
target_link_libraries(${PROJECT_NAME}.elf 
    cmsis hal 
    lan8742 lwip 
    cmsis-rtos
    wolfssl wolfcrypttest_lib wolfcryptbench_lib 
    wolfssh 
    cjson
    cjson_utils
    -lc -lm -lnosys )
    
set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map")
set(HEX_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")

add_custom_target(flush
    COMMAND  openocd -f ${CMAKE_CURRENT_SOURCE_DIR}/openocd.cfg -c "program ${CMAKE_BINARY_DIR}/tty2eth.elf verify reset exit"
    DEPENDS  ${PROJECT_NAME}.elf
)