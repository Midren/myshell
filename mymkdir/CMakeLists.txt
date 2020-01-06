cmake_minimum_required(VERSION 3.14)
project(mymkdir)

set(CMAKE_CXX_STANDARD 17)

add_executable(mymkdir main.cpp)
install (TARGETS mymkdir EXPORT mymkdir RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}../bin)
export(EXPORT mymkdir FILE "${CMAKE_CURRENT_BINARY_DIR}/../mymkdirTargets.cmake")