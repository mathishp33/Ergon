cmake_minimum_required(VERSION 3.31)
project(Ergon)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(Talos)
add_subdirectory(tests/test_0)
add_subdirectory(tests/test_1)
add_subdirectory(tests/test_2)
add_subdirectory(tests/test_3)