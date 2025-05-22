# block(SCOPE_FOR VARIABLES)

file(GLOB HAL_SRCS external/stm32h7xx_hal_driver/Src/*.c)
list(FILTER HAL_SRCS EXCLUDE REGEX ".*template\\.c$")

add_library(hal ${HAL_SRCS})
target_include_directories(hal PUBLIC "external/stm32h7xx_hal_driver/Inc" "include/hal")
target_link_libraries(hal cmsis)

# endblock()