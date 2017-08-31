cmake_minimum_required(VERSION 2.8)

project(sliding_doors)

add_executable(sliding_doors main.cpp)

install(TARGETS sliding_doors RUNTIME DESTINATION bin)
