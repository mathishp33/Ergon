cmake_minimum_required(VERSION 4.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ergon_test_sys_read
        sys_read.cpp
)

target_link_libraries(ergon_test_sys_read
        PRIVATE
        talos
)

add_test(NAME ErgonTest_Sys_Read COMMAND ergon_test_sys_read)