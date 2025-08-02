# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(spark_tests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} container_tests.c)
