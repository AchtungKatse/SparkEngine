# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(spark_tests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup engine
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH spark_dir)
message("Spark dir: ${spark_dir}")

set(SPARK_DEBUG ON)
set(SPARK_LINK_STATIC ON)
set(SPARK_CORE_ONLY ON)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_INPORTERS_BY_DEFAULT OFF)
add_subdirectory(${spark_dir} ${CMAKE_BINARY_DIR}/engine)

add_executable(hashmap_tests hashmap_tests.c)
target_link_libraries(hashmap_tests PRIVATE SparkCore)
target_include_directories(hashmap_tests PRIVATE "${spark_dir}/include")

add_executable(allocator_tests allocator_tests.c)
target_link_libraries(allocator_tests PRIVATE SparkCore)
target_include_directories(allocator_tests PRIVATE "${spark_dir}/include")

add_executable(noise noise.c)
target_link_libraries(noise PRIVATE SparkCore)
target_include_directories(noise PRIVATE "${spark_dir}/include" "${spark_dir}/dependencies/stb")
