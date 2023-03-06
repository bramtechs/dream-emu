cmake_minimum_required(VERSION 3.0)
project(temple CXX C)
set(CMAKE_C_STANDARD 11)

FILE(GLOB THESAUCE src/*.c src/*.h)
if (WIN32)
    message("Stripping Linux code")
else()
    message("Stripping Windows code")
    list(REMOVE_ITEM THESAUCE ${CMAKE_CURRENT_SOURCE_DIR}/src/client_win.c)
endif ()

# Declaring our executable
add_executable(${PROJECT_NAME} WIN32 ${THESAUCE})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC magma)
