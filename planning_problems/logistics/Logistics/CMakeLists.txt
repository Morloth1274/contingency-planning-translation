cmake_minimum_required(VERSION 2.8)

project(logistics)

add_executable(logistics main.cpp)

install(TARGETS logistics RUNTIME DESTINATION bin)
