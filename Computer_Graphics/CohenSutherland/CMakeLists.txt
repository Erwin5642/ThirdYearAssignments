cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(cohen_sutherland cohen_sutherland.cpp)

target_link_libraries(cohen_sutherland freeglut opengl32 glu32)

