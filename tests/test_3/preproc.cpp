#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(

    .section .text
     %define SIZE 64
     %macro INC(reg)
      addi reg, reg, 1
     %endmacro
     ldw r0, var
     INC(r0)
     %rep 4*1
      inc r0
     %endrep
    .section .data
      ; %assign a 5
      ; %assign a a + 5
      ; %assign a a * 2 + (6 == 4)
     var:
       .word SIZE

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

    std::cout << "REGISTER R0: " << env_m.get_from_reg("r0") << std::endl;



    return 0;
}