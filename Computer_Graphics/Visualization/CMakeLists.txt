cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(planes planes.c)

target_link_libraries(planes freeglut opengl32 glu32)

