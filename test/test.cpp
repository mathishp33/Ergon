#include <iostream>

#include "../Talos/include/env_manager.h"

#include <string>

int main() {
    std::cout << "---------- TEST PROGRAM ----------\n";
    std::string program = "var a 1 \n"
                          "load a r3 r2 \n"
                          "addi 2 3 30";
    std::cout << program << std::endl;

    auto env_m = EnvironmentManager<0x10000, 0xFFF>();
    env_m.build(program);

    std::cout << "\n---------- INTERPRETER RESULT ----------\n";

    std::cout << "ERROR: " << env_m.get_error_msg() << std::endl;

    return 0;
}