#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>
#include <filesystem>


namespace fs = std::filesystem;

std::string load(const fs::path& base_path, const fs::path& path) {
    fs::path full_path = base_path / path;
    const std::ifstream file(full_path.string());

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
    auto env = EnvironmentManager();

    std::cout << "\n---------- BUILD RESULT ----------\n";

    const fs::path BASE_PATH = { "C:/Users/mathi/CLionProjects/Ergon/tests/test_kernel/vm/" };
    const std::vector<std::pair<std::string, std::string>> incs = {
        { "mmio.inc", load(BASE_PATH, "mmio.inc") }, { "layout.inc", load(BASE_PATH, "layout.inc") },
        { "syscalls.inc", load(BASE_PATH, "syscalls.inc") } };
    auto pool = [&](std::vector<std::pair<std::string, std::string>> v) {
        v.insert(v.end(), incs.begin(), incs.end()); return v; };

    std::cout << env.build_rom(pool({ { "boot.asm", load(BASE_PATH, "boot.asm") } })) << std::endl;
    std::cout << env.build_disk(pool({ { "kernel.asm", load(BASE_PATH, "kernel.asm") },
        { "storage.asm", load(BASE_PATH, "storage.asm") },{ "loader.asm", load(BASE_PATH, "loader.asm") } }),
        { { "hello", pool({ { "hello.asm", load(BASE_PATH, "hello.asm") } }) } })
    << std::endl;

    env.mb.reset();

    const auto start = std::chrono::high_resolution_clock::now();
    const RunResult rr = env.start();
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";

    std::cout << "RUN DURATION: " << duration.count() << " micro_sec\n" << std::endl;

    std::cout << "RunResult: " << to_str(rr) << "\n";

    std::cout << "EXIT CODE:  " << env.exit_code << std::endl;

    return 0;
}