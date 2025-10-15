cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(chaotic_function chaotic_function.cpp)

target_link_libraries(chaotic_function freeglut opengl32 glu32)

