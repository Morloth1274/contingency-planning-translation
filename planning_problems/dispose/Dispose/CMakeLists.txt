cmake_minimum_required(VERSION 2.8)

project(dispose)

add_executable(dispose main.cpp)

install(TARGETS dispose RUNTIME DESTINATION bin)
