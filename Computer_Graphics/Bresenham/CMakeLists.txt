cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(bresenham bresenham.cpp)

target_link_libraries(bresenham freeglut opengl32 glu32)