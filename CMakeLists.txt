cmake_minimum_required(VERSION 3.14)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Curses REQUIRED)
add_subdirectory(mycat)
include_directories(${Curses_INCLUDE_DIR})
add_executable(myshell main.cpp Shell.cpp Command.cpp Token.cpp util.cpp)
install(EXPORT mycat DESTINATION /Users/admin/Desktop/POK/myshell/bin)
target_link_libraries(myshell ${CURSES_LIBRARIES})
