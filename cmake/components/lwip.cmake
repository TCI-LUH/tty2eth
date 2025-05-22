# block(SCOPE_FOR VARIABLES)

file(GLOB_RECURSE LWIP_SRCS 
    external/stm32_mw_lwip/src/api/*.c
    external/stm32_mw_lwip/src/core/*.c
    external/stm32_mw_lwip/src/netif/*.c
    external/stm32_mw_lwip/system/OS/*.c
    external/stm32_mw_lwip/src/apps/sntp/*.c
    )

add_library(lwip ${LWIP_SRCS})

target_include_directories(lwip PUBLIC 
    "external/stm32_mw_lwip/src/include"
    "external/stm32_mw_lwip/system"
    "include/lwip"
    )
target_link_libraries(lwip hal lan8742 cmsis-rtos)


# endblock()