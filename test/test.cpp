#include <iostream>

#include "../Talos/include/env_manager.h"

#include <string>
#include <chrono>

int main() {
    std::cout << "---------- TEST PROGRAM ----------\n";
    std::string program = "rmax dd 2_000_000 \n"
                          "ldw eax rmax       \n"
                          "loop_start:       \n"
                          " addi ecx ecx 1     \n"
                          " cmpu ecx eax       \n"
                          " jl loop_start    \n"
                          ""
                          "halt              \n";

    std::cout << program << std::endl;

    auto env_m = EnvironmentManager<0xFFFF>();

    env_m.build(program);

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << "ERROR: " << env_m.get_error_msg() << std::endl;

    //running the program
    env_m.set_mod(RunMode::AUTO);

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;
    return 0;
}