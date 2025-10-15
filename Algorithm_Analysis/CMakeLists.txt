cmake_minimum_required(VERSION 3.29)
project(Algorithm_Analysis)

set(CMAKE_CXX_STANDARD 20)

add_executable(breath_first_search breath_first_search.c)

add_compile_options(-Wall -Wextra -Wpedantic)