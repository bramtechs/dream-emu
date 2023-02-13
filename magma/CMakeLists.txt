cmake_minimum_required(VERSION 3.0)
project(magma CXX C)

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

# Customize raylib
set(CUSTOMIZE_BUILD         ON CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_JPG  ON CACHE BOOL "" FORCE)

set(SUPPORT_GESTURES_SYSTEM     OFF CACHE BOOL "" FORCE)
set(SUPPORT_MOUSE_GESTURES      OFF CACHE BOOL "" FORCE)
set(SUPPORT_SSH_KEYBOARD_RPI    OFF CACHE BOOL "" FORCE)

# temporarely for temple
set(SUPPORT_MODULE_RMODELS      OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_OBJ      OFF CACHE BOOL "" FORCE)
set(MAGMA_3D                    OFF CACHE BOOL "" FORCE)

#set(SUPPORT_SCREEN_CAPTURE "Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)
#set(SUPPORT_GIF_RECORDING "Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)

add_subdirectory(src)
add_subdirectory(box2d)
add_subdirectory(tools)

add_subdirectory(raylib)
