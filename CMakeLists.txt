cmake_minimum_required(VERSION 3.14)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Curses REQUIRED)

include_directories(${Curses_INCLUDE_DIR})
add_executable(myshell main.cpp Shell.cpp Command.cpp Token.cpp util.cpp)

target_link_libraries(myshell ${CURSES_LIBRARIES})
