cmake_minimum_required(VERSION 3.20)
project(cathack C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)

include_directories(include ${CURSES_INCLUDE_DIR})

file(GLOB SOURCES "src/*.c")

add_executable(cathack ${SOURCES})
target_link_libraries(cathack PRIVATE ${CURSES_LIBRARIES})
