# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(spark_benchmarks)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} benchmarks.c)
target_compile_options(${PROJECT_NAME} PRIVATE "-g")
