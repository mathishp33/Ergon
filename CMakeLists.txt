cmake_minimum_required(VERSION 4.1)
project(Ergon)

set(CMAKE_CXX_STANDARD 20)

option(ERGON_BUILD_TESTS "Build tests" ON) #add test dir

add_library(talos INTERFACE)
target_include_directories(talos INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Talos
)
target_link_libraries(talos INTERFACE ergon_common)

add_library(hephaistos INTERFACE)
target_include_directories(hephaistos INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Hephaistos
)
target_link_libraries(hephaistos INTERFACE ergon_common)

if (ERGON_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()