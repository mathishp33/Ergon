#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(

    .section .text
      %define SIZE 64
      %macro INC(reg)
       addi reg, reg, 1
      %endmacro
      ldw r1, var
      INC(r1)
      %rep 4*1
       inc r1
      %endrep
      %if 1
        inc r1
      %else
        inc r2
      %endif
      %ifdef S
        inc r2
      %endifdef
      %ifdef SIZE
        inc r2
      %endifdef
      movi r0, 5
      halt
    .section .data
      var:
        .word SIZE

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

    std::cout << "R0: " << (int)env_m.get_from_reg("r0") << std::endl;
    std::cout << "R1: " << (int)env_m.get_from_reg("r1") << std::endl;
    std::cout << "R2: " << (int)env_m.get_from_reg("r2") << std::endl;



    return 0;
}