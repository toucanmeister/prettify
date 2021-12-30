cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(prettify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(prettify main.cpp)