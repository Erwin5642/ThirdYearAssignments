cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(pickline pickline.cpp)

target_link_libraries(pickline freeglut opengl32 glu32)
