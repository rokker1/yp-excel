cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
# set the project name
project(yp-excel)

# add the executable
add_executable(excel src/main.cpp src/structures.cpp)