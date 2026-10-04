#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>


int main() {
   std::string program = R"(

    .section .text
      .entry main
      main:
        movi r10, 0xF000
        shli r10, r10, 16 ; r10 = MMIO_BASE
        movi r1, 72 ; 'H'
        sbaseb r1, r10, 0 ; CONSOLE_OUT
        movi r1, 10 ; '\n'
        sbaseb r1, r10, 0
        movi r0, 42 ; code de sortie
        halt
)";

    auto env_m = EnvironmentManager();

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << env_m.build({ { "main", program } }) << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";

    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;

    std::cout << "EXIT CODE:  " << env_m.exit_code << std::endl;
    std::cout << "R0:  " << env_m.get_from_reg("r0") << std::endl;



    return 0;
}