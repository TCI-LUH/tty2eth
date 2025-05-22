# block(SCOPE_FOR VARIABLES)

add_library(lan8742 "external/stm32-lan8742/lan8742.c")
target_include_directories(lan8742 PUBLIC "external/stm32-lan8742/")

# endblock()