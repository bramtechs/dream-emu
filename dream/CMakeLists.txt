cmake_minimum_required(VERSION 3.0)
project(dream C)
set(CMAKE_C_STANDARD 99)

# Declaring our executable
FILE(GLOB THESAUCE src/*.c src/*.h) # TODO put ./shared into own library
add_executable(${PROJECT_NAME} ${THESAUCE})
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC magma)
