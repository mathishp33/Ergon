#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(

    .section .text
      movi r0, 1 ; syscall = 1 (write)
      lea r1, r6, my_char_buff ; buffer
      ldb r2, my_size ; buffer_size
      syscall

    .section .data
      my_char_buff:
        .byte 'a', 'b', 'c', 'd'
      my_size:
        .word 4

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

    std::cout << "EXIT CODE:  " << env_m.exit_code << std::endl;



    return 0;
}