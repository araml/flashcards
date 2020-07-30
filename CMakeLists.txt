cmake_minimum_required(VERSION 3.0)

project(flashcard_px)

enable_testing()
find_library(SDL2 SDL2)

set(CMAKE_CXX_FLAGS "-ggdb -std=c++2a -Wall -Werror -Wextra -Wsign-conversion")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options (-fcolor-diagnostics)
endif ()


macro(add_files name)
    file(GLOB ${name} src/${name}/*.h src/${name}/*.cpp src/${name}/*.c)
    include_directories(src/${name})
endmacro()

set(sqlite "${CMAKE_CURRENT_SOURCE_DIR}/sqlite")

include_directories(${sqlite})
include_directories(src)
#include_directories(ncurses_mock)

add_executable(flashcard_px src/main.cpp src/sql.cpp )
#set_source_files_properties(ncurses_mock/ncurses.c PROPERTIES LANGUAGE CXX )

target_link_libraries(flashcard_px "pthread" "ncursesw" "dl" "GL" "GLU" "rt")
target_link_libraries(flashcard_px "${sqlite}/sqlite.a" "pthread")

# if (${NCURSES_MOCK})


# Tests
#add_test(test_sql test_sql)
#add_executable(test_sql tests/test_sql.cpp src/sql.cpp)
#target_link_libraries(test_sql "${sqlite}/sqlite.a" "pthread" "dl" "rt" )
