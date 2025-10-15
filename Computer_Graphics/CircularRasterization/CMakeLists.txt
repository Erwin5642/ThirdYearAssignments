cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(circular_rasterization circular_rasterization.cpp)

target_link_libraries(circular_rasterization freeglut opengl32 glu32)

