cmake_minimum_required(VERSION 4.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ergon_test_stack
        stack.cpp
)

target_link_libraries(ergon_test_stack
        PRIVATE
        talos
        #hephaistos
)

add_test(NAME ErgonTest_Stack COMMAND ergon_test_stack)