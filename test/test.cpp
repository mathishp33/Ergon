#include <iostream>

#include "../Talos/include/env_manager.h"

#include <string>

int main() {
    std::cout << "---------- TEST PROGRAM ----------\n";
    std::string program = "a DB 1 ; define variable 'a' with a size of 1 byte and a initial value of 1\n"
                          "load a r3 \n"
                          "addi r2 r3 30";
    std::cout << program << std::endl;

    auto env_m = EnvironmentManager<0x10000, 0xFFF>();

    env_m.build(program);
    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << "ERROR: " << env_m.get_error_msg() << std::endl;

    //running the program
    env_m.set_mod(RunMode::AUTO);
    env_m.start();

    std::cout << "\n---------- REGS CONTENTS ----------\n";

    std::cout << "regs[2] = " << env_m.get_from_reg(2) << std::endl;
    std::cout << "regs[3] = " << env_m.get_from_reg(3) << std::endl;

    return 0;
}