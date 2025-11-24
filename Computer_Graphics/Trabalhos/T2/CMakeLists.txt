cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 11)

add_executable(bezier bezier.cpp)

target_link_libraries(bezier freeglut opengl32 glu32)

