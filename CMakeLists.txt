cmake_minimum_required(VERSION 3.20)
project(complie)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-DLOCAL)
add_executable(complie main.cpp getword.h symbol_table.h Output_region.h)
