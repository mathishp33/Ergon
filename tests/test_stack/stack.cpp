#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(
%macro proc()
    push fp
    mov fp, sp
    %assign __frame_size 0
%endmacro

%macro local(name, size)
    subi sp, sp, size
    %assign __frame_size __frame_size + size
    %assign name -__frame_size
%endmacro

%macro endproc()
    mov sp, fp
    pop fp
    ret
%endmacro

.section .text

.global main
.entry main

main:
    call test_local
    halt


test_local:
    proc()

    local(x, 4)

    movi r0, 42
    sbasew r0, fp, x

    lbasew r1, fp, x

    ; r1 doit valoir 42

    endproc()
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

    std::cout << "r1: " << (int)env_m.get_from_reg("r1") << std::endl;

    return 0;
}