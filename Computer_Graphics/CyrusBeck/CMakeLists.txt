cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

add_executable(cyrus_beck cyrus_beck.cpp)

target_link_libraries(cyrus_beck freeglut opengl32 glu32)

