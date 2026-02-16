cmake_minimum_required(VERSION 4.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ergon_tests
        test.cpp
)

target_link_libraries(ergon_tests
        PRIVATE
        talos
        #hephaistos
)

add_test(NAME ErgonTests COMMAND ergon_tests)