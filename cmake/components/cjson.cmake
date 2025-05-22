option(ENABLE_CJSON_TEST "" OFF)
option(CJSON_BUILD_SHARED_LIBS "" OFF)
option(ENABLE_CJSON_UTILS "" ON)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/cJSON")
target_include_directories(cjson PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON>)