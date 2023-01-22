cmake_minimum_required(VERSION 3.0)
project(temple CXX C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Declaring our executable
FILE(GLOB THESAUCE src/*.cpp src/*.h) # TODO put ./shared into own library
add_executable(${PROJECT_NAME} WIN32 ${THESAUCE})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC magma)
