cmake_minimum_required(VERSION 3.0)
project(magma CXX C)

cmake_policy(SET CMP0054 NEW)

# Adding Raylib
set(BUILD_EXAMPLES OFF) # don't build the supplied examples
set(BUILD_GAMES    OFF) # don't build the supplied example games

# Customize raylib
set(CUSTOMIZE_BUILD         ON)
set(SUPPORT_FILEFORMAT_JPG  ON)

include(CMakeDebloat.txt)

option(MAGMA_3D "Build 3D functionality." OFF)
option(MAGMA_VIDEO "Build video player."  OFF)

if (MAGMA_3D)
    set(SUPPORT_MODULE_RMODELS      ON)
    message("Compiling 3D support")
else()
    set(SUPPORT_MESH_GENERATION     OFF)
    set(SUPPORT_MODULE_RMODELS      OFF)
    set(SUPPORT_FILEFORMAT_OBJ      OFF)
    set(SUPPORT_FILEFORMAT_MTL      OFF)
    set(SUPPORT_FILEFORMAT_IQM      OFF)
    set(SUPPORT_FILEFORMAT_GLTF     OFF)
    set(SUPPORT_FILEFORMAT_VOX      OFF)
    message("Skipping 3D support")
endif()

if (MAGMA_VIDEO)
    message("Compiling video support")
else()
    message("Skipping video support")
endif()

#set(SUPPORT_SCREEN_CAPTURE "Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)
#set(SUPPORT_GIF_RECORDING "Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)

add_subdirectory(src)
add_subdirectory(box2d)
add_subdirectory(tools)

add_subdirectory(raylib)
