cmake_minimum_required(VERSION 3.0)
project(deflation_cli CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

# Declaring our executable
FILE(GLOB THESAUCE *.cpp *.h)
add_executable(${PROJECT_NAME} ${THESAUCE})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE} src)
target_link_libraries(${PROJECT_NAME} PUBLIC deflation)
target_link_libraries(${PROJECT_NAME} PUBLIC magma)
