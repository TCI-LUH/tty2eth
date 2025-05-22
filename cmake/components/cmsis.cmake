# block(SCOPE_FOR VARIABLES)

# add_definitions("-DCMSIS_device_header=<stm32h7xx.h>")
add_library(cmsis INTERFACE)
target_include_directories(cmsis INTERFACE 
    "external/cmsis_core/Include"
    "external/cmsis_core/RTOS2/Include"
    "external/cmsis_core/Core_A/Include"
    "external/cmsis_device_h7/Include")

# endblock()