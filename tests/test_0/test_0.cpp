#include "../../Talos/include/environment_manager.h"

#include <iostream>
#include <string>
#include <chrono>

int main() {
    std::string program = ".section .text \n"
                          " ldw r0, var \n"
                          " loop: \n"
                          "  inc r1 \n"
                          "  cmp r0, r1 \n"
                          "  jz loop \n"
                          ".section .data \n"
                          " var: \n"
                          "   .word 200_000_000\n";

    auto env_m = EnvironmentManager(0xFFFFFFFF);

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