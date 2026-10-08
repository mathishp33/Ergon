#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>
#include <filesystem>


namespace fs = std::filesystem;

std::string load(const fs::path& path) {
    const std::ifstream file(fs::absolute(path).string());

    if (!file)
        return "error";

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

static const char* to_str(const RunResult r) {
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

    const fs::path BOOT_PATH = { "vm/boot.asm" };
    const fs::path KERNEL_PATH = { "vm/kernel.asm" };

    const std::string boot = load(BOOT_PATH);
    const std::string kernel = load(KERNEL_PATH);

    std::cout << env.build_rom({ { "boot.asm", boot } }) << std::endl;
    std::cout << env.install_kernel({ { "kernel.asm", kernel } }) << std::endl;

    env.mb.reset();

    const auto start = std::chrono::high_resolution_clock::now();
    const RunResult rr = env.start();
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";

    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;

    std::cout << "\n" << to_str(rr) << ", exit_code=" << env.exit_code << "\n";

    std::cout << "EXIT CODE:  " << env.exit_code << std::endl;

    return 0;
}