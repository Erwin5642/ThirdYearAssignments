cmake_minimum_required(VERSION 3.29)
project(Computer_Graphics)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(CohenSutherland/)

add_subdirectory(CyrusBeck/)

add_subdirectory(ChaoticFunction/)

add_subdirectory(DDA/)

add_subdirectory(Bresenham/)

add_subdirectory(CircularRasterization/)

add_subdirectory(AnimationFunction/)

add_subdirectory(Prova/)
