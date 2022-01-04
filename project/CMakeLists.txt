cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(prettify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

find_package(OpenMP REQUIRED)

add_executable(prettify main.cpp prettify.cpp)
target_link_libraries(prettify PUBLIC OpenMP::OpenMP_CXX)

enable_testing()
add_executable(prettify_test prettify_test.cpp prettify.cpp)
target_link_libraries(prettify_test PUBLIC OpenMP::OpenMP_CXX)
add_test(Read_Img prettify_test 1)
add_test(Write_Img prettify_test 2)