#include "../../Talos/include/environment_manager.h"

#include <iostream>
#include <string>
#include <chrono>

int main() {
    std::string program = ".section .text \n"
                          " fldw f0, var \n"
                          " fldw f2, increment \n"
                          " loop: \n"
                          "  fadd f1, f1, f2 \n"
                          "  fcmp f0, f1 \n"
                          "  jz loop \n"
                          ".section .data \n"
                          " var: \n"
                          "   .word 0x41200000 ; 10.0 en float\n"
                          " increment: \n"
                          "   .word 0x3F800000 ; 1.0 en float\n";

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