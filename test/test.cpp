#include <iostream>

#include "../Talos/include/env_manager.h"

#include <string>

void debug(EnvironmentManager<0x10000, 0xFFF>& env) {
    std::cout << "\n---------- PC ----------\n";
    env.step();
    std::cout << "PC = " << env.get_from_reg(15) << std::endl;

    std::cout << "\n---------- REGS CONTENTS ----------\n";
    std::cout << "ram[0] = " << env.get_from_ram(0) << std::endl;
    std::cout << "regs[2] = " << env.get_from_reg(2) << std::endl;
    std::cout << "regs[3] = " << env.get_from_reg(3) << std::endl;
}

int main() {
    std::cout << "---------- TEST PROGRAM ----------\n";
    std::string program = "a DB 1 ; define variable 'a' with a size of 1 byte and a initial value of 1\n"
                          "ldb r3 a \n"
                          "addi r2 r3 30";
    std::cout << program << std::endl;

    auto env_m = EnvironmentManager<0x10000, 0xFFF>();

    env_m.build(program);

    std::cout << "\n---------- BUILD RESULT ----------\n";
    std::cout << "ERROR: " << env_m.get_error_msg() << std::endl;

    //running the program
    env_m.set_mod(RunMode::STEP);

    env_m.start();

    debug(env_m);
    debug(env_m);
    debug(env_m);

    return 0;
}