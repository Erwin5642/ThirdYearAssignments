cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(light light.c)

target_link_libraries(light freeglut opengl32 glu32)

