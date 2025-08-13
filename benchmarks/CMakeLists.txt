# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(spark_benchmarks)
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

# add benchmarks
add_executable(dynamic_allocator dynamic_allocator.c)
target_link_libraries(dynamic_allocator PRIVATE SparkCore)
target_include_directories(dynamic_allocator PRIVATE "${spark_dir}/include")

add_executable(noise noise.c)
target_link_libraries(noise PRIVATE SparkCore)
target_include_directories(noise PRIVATE "${spark_dir}/include")
target_compile_options(noise PRIVATE "-g")
# target_compile_options(noise PRIVATE "-fprofile-generate")
# target_link_libraries(noise PRIVATE gcov)
