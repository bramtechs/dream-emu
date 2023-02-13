cmake_minimum_required(VERSION 3.0)
project(magma CXX C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/editor_utils.cpp)

file(GLOB HEADER_FILES *.h *.hpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE} .)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib tobanteGaming::Box2D)

option(MAGMA_3D "Include magma 3d features" ON)
