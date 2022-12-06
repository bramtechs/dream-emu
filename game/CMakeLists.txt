cmake_minimum_required(VERSION 3.0)
project(dreamemu_game CXX C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} SHARED game.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
include_directories("../shared")
