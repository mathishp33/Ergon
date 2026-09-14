#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(

    .section .text

    movi r0, 2
    leab r1, buffer
    movi r2, 5
    syscall

    mov r4, r0

    call print

    movi r0, 0 ; syscall = 0 (exit)
    syscall

    print:
      movi r0, 1
      leab r1, buffer
      mov r2, r4
      syscall
      ret


    .section .bss
      buffer:
        .zero 16
)";

    /*
     {length 13, capacity 13}


     [5]  = 92 00 00 00 02 (call)
     [6]  =
     [7]  = 94 00 00 00 00 (syscall)
     [8]  = 60 00 00 00 01 (movi)
     [9]  = 83 01 00 00 00 (leab)
     [10] = 61 02 04 00 00 (mov)
     [11] = 94 00 00 00 00 (syscall)
     [12] = 93 00 00 00 00 (ret)
     */


    auto env_m = EnvironmentManager(64);

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