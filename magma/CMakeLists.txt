cmake_minimum_required(VERSION 3.0)
project(magma CXX C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "glfw" CACHE PATH "path of glfw")

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

#set(SUPPORT_SCREEN_CAPTURE "Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)
#set(SUPPORT_GIF_RECORDING "Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()" ON CUSTOMIZE_BUILD ON)

add_subdirectory(box2d)
add_subdirectory(tools)

add_subdirectory(raylib)

# Declaring our executable
FILE(GLOB THESAUCE src/*.cpp src/*.h)
add_library(${PROJECT_NAME} STATIC ${THESAUCE})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE} src)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib tobanteGaming::Box2D)

option(MAGMA_3D "Include magma 3d features" OFF)
