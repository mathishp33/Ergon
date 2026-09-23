#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>


int main() {
    std::string program = R"(
.section .text

  .global main
  .entry main
  main:

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

    return 0;
}