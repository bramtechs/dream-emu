cmake_minimum_required(VERSION 3.0)
project(space CXX C)
set(CMAKE_C_STANDARD 99)

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

# Declaring our executable
add_executable(${PROJECT_NAME} client.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
include_directories("../shared")
