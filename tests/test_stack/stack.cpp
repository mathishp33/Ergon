#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string macros = R"(
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
)";
    std::string program = macros + R"(
.section .text

.global main
.entry main

main:
    call test_locals
    halt


test_locals:
    proc()

    local(a, 4)
    local(b, 4)
    local(c, 4)
    local(d, 4)

    movi r0, 10
    sbasew r0, fp, a

    movi r0, 20
    sbasew r0, fp, b

    movi r0, 30
    sbasew r0, fp, c

    movi r0, 40
    sbasew r0, fp, d

    lbasew r1, fp, a
    lbasew r2, fp, b
    lbasew r3, fp, c

    add r0, r1, r2
    add r0, r0, r3

    ; r0 = 60

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

    for (int i = 0; i < 9; i ++)
        std::cout << "r" << i << ": " << (int)env_m.get_from_reg("r" + std::to_string(i)) << std::endl;

    return 0;
}