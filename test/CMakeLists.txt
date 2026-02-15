

add_executable(ergon_tests
        test.cpp
)

target_link_libraries(ergon_tests
        PRIVATE
        talos
        hephaistos
)

add_test(NAME ErgonTests COMMAND ergon_tests)