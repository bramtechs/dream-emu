cmake_minimum_required(VERSION 3.0)
project(magma CXX C)

cmake_policy(SET CMP0054 NEW)

# Adding Raylib
set(BUILD_EXAMPLES OFF) # don't build the supplied examples
set(BUILD_GAMES    OFF) # don't build the supplied example games

# Customize raylib
set(CUSTOMIZE_BUILD         ON)
set(SUPPORT_FILEFORMAT_JPG  ON)
set(SUPPORT_FILEFORMAT_JPG  ON)
include(CMakeDebloat.txt)

add_subdirectory(src)
add_subdirectory(box2d)
add_subdirectory(tools)

add_subdirectory(raylib)
