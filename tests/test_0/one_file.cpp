#include "../../Talos/include/environment_manager.h"

#include <iostream>
#include <string>
#include <chrono>

int main() {
    std::string program = R"(

    .section .text
     fldw f0, var
     fldw f2, increment
     loop:
       fadd f1, f1, f2
       fcmp f0, f1
       jz loop
    .section .data
     var:
       .word 10.0
     increment:
       .word 1.0


)";

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