cmake_minimum_required(VERSION 3.0)

project(flashcard_px)

find_library(SDL2 SDL2)

set(CMAKE_CXX_FLAGS "-ggdb")

macro(add_files name)
    file(GLOB ${name} src/${name}/*.h src/${name}/*.cpp)
    include_directories(src/${name})
endmacro()

set(sqlite "${CMAKE_CURRENT_SOURCE_DIR}/sqlite")

include_directories(${sqlite})

add_executable(flashcard_px src/main.cpp)

target_link_libraries(flashcard_px "ncursesw" "pthread" "dl" "GL" "GLU" "rt")

target_link_libraries(flashcard_px "${sqlite}/sqlite.a" "pthread")
