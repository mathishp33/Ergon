#include <iostream>

#include "../../Talos/include/env_manager.h"

#include <string>
#include <chrono>

int main() {
    std::string lib = ".text \n"
                      ".global add_two \n"
                      "add_two: \n"
                      "  addi eax eax 2 \n"
                      "  ret \n";
    std::string main = ".entry main \n"
                       ".extern add_two \n"
                       ".text \n"
                       ".global main \n"
                       "main: \n"
                       "  call add_two \n"
                       "  halt \n";

    auto env_m = EnvironmentManager<0xFFFF>();

    std::cout << "\n---------- BUILD RESULT ----------\n";
    std::vector files = { lib, main };
    std::cout << "ERROR: " << env_m.build_lib(files) << std::endl;


    //running the program
    env_m.set_mod(ExecMode::AUTO);

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;

    std::cout << "content of eax: " << env_m.get_from_reg("eax") << std::endl;
    return 0;
}