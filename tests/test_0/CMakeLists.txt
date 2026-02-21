cmake_minimum_required(VERSION 4.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ergon_test_0
        test_0.cpp
)

target_link_libraries(ergon_test_0
        PRIVATE
        talos
        #hephaistos
)

add_test(NAME ErgonTest_0 COMMAND ergon_test_0)