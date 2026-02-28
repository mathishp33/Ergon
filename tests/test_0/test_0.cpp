#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>

int main() {
    std::string program = "section .text \n"
                          " ldw r0 var \n"
                          " loop: \n"
                          "  inc r1 \n"
                          "  cmp r1 r0 \n"
                          "  jl loop \n"
                          " stb "
                          "section .data \n"
                          " var dd 20_000_000 \n"
                          "section .rodata \n"
                          " var2 db 4\n";

    EnvironmentManager env_m = EnvironmentManager(0xFFFFFFFF);

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << env_m.build({ { "main", program } }) << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;
    return 0;
}