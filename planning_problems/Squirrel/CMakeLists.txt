cmake_minimum_required(VERSION 2.8)

project(squirrel)

add_executable(squirrel main.cpp)

install(TARGETS squirrel RUNTIME DESTINATION bin)
