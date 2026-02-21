#include <iostream>

#include "../../Talos/include/env_manager.h"

#include <string>
#include <chrono>

int main() {
    std::string program = ".text \n"
                          " ldw ebx var \n"
                          " loop: \n"
                          "  inc eax \n"
                          "  cmp eax ebx \n"
                          "  jl loop \n"
                          ".data \n"
                          " var dd 2_000_000 \n"
                          ".bss \n"
                          " buffer times 256 resb";

    EnvironmentManager<0xFFFF> env_m = EnvironmentManager<0xFFFF>();

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << env_m.build_single(program) << std::endl;

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