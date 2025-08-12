# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(spark_benchmarks)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} benchmarks.c)
# target_compile_options(${PROJECT_NAME} PRIVATE "-g")
# target_compile_options(${PROJECT_NAME} PRIVATE "-fprofile-use")
target_link_libraries(${PROJECT_NAME} PRIVATE gcov)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_INPORTERS_BY_DEFAULT OFF)
