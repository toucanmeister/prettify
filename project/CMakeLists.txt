cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(prettify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

find_package(OpenMP REQUIRED)

add_executable(prettify)
target_include_directories(prettify PRIVATE source)
target_sources(prettify PRIVATE source/main.cpp PRIVATE source/prettify.cpp)
target_link_libraries(prettify PUBLIC OpenMP::OpenMP_CXX)

enable_testing()
add_executable(prettify_test)
target_include_directories(prettify_test PRIVATE source PRIVATE test)
target_sources(prettify_test PRIVATE source/prettify.cpp PRIVATE test/prettify_test.cpp)
target_link_libraries(prettify_test PUBLIC OpenMP::OpenMP_CXX)
add_test(Read_Img prettify_test 1)
add_test(Write_Img prettify_test 2)
add_test(Mean_Filter prettify_test 3)
add_test(Gauss_Filter prettify_test 4)
add_test(Median_Filter prettify_test 5)
add_test(Threshold_Mean prettify_test 6)