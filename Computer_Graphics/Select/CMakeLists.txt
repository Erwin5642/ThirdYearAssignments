cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(select select.cpp)

target_link_libraries(select freeglut opengl32 glu32)
