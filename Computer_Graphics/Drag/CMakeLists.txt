cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(drag drag.cpp)

target_link_libraries(drag freeglut opengl32 glu32)

