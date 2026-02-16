cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(talos INTERFACE)

target_include_directories(talos INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(talos INTERFACE cxx_std_20)