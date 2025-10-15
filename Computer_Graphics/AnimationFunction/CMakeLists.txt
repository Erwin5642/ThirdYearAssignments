cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(animation_function animation_function.cpp)

target_link_libraries(animation_function freeglut opengl32 glu32)