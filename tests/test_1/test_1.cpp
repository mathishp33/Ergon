#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>

int main() {
    std::string file1 =
        ".section .text \n"
        " .extern func2 \n"
        " .global main \n"
        " main: \n"
        "  movi r0, 10 \n"
        "  call func2 \n"
        "  stb r0, var1 \n"
        "  halt \n"
        " .entry main \n"
        ".section .data \n"
        " var1: \n"
        "  .word 0x1234 \n";

    std::string file2 =
        ".section .text \n"
        " .global func2 \n"
        " func2: \n"
        "  movi r1, 42 \n"
        "  ret \n"
        ".section .bss \n"
        " buffer: \n"
        "  .space 16 \n";

    auto env_m = EnvironmentManager(0xFFFFFFFF);

    std::cout << "\n---------- BUILD RESULT ----------\n";
    std::vector<std::pair<std::string, std::string>> files = { { "file1", file1 }, { "file2", file2 } };
    std::cout << "ERROR: " << env_m.build(files) << std::endl;


    //running the program
    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;

    return 0;
}