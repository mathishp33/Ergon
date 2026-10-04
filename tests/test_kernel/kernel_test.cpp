#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>


namespace fs = std::filesystem;

std::string load(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        return "error";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

static const char* to_str(RunResult r) {
    switch (r) {
        case RunResult::HALTED: return "HALTED";
        case RunResult::STACK_OVERFLOW: return "STACK_OVERFLOW";
        case RunResult::STACK_UNDERFLOW: return "STACK_UNDERFLOW";
        case RunResult::SYSCALL_STOP: return "SYSCALL_STOP";
        case RunResult::ERROR: return "ERROR";
        case RunResult::PC_OVERFLOW: return "PC_OVERFLOW";
    }
    return "?";
}

int main() {
    auto env = EnvironmentManager(0xFFFFFFFF);

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::string boot = load("C:/Users/mathi/CLionProjects/Ergon/tests/test_kernel/boot.asm");
    std::string kernel = load("C:/Users/mathi/CLionProjects/Ergon/tests/test_kernel/kernel.asm");

    std::cout << env.build_rom({ { "boot.asm", boot } }) << std::endl;
    std::cout << env.install_kernel({ { "kernel.asm", kernel } }) << std::endl;

    env.mb.reset();

    auto start = std::chrono::high_resolution_clock::now();
    RunResult rr = env.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";

    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;

    std::cout << "\n" << to_str(rr) << ", exit_code=" << env.exit_code << "\n";

    std::cout << "EXIT CODE:  " << env.exit_code << std::endl;

    return 0;
}