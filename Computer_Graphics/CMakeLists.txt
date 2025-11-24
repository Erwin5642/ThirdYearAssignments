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

add_subdirectory(Visualization/)

add_subdirectory(Light/)

add_subdirectory(MoveLight/)

add_subdirectory(Material/)

add_subdirectory(Select/)

add_subdirectory(Pickline/)

add_subdirectory(Drag/)

add_subdirectory(Trabalhos/T2/)