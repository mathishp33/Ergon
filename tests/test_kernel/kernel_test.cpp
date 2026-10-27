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
        return "file not found";

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

    const fs::path BASE_PATH = { "C:/Users/Utilisateur/CLionProjects/Ergon/tests/test_kernel" };
    const fs::path KERNEL_PATH = BASE_PATH / "vm/kernel";
    const fs::path BOOT_PATH = BASE_PATH / "vm/boot";
    const fs::path INCS_PATH = BASE_PATH / "vm/includes/";
    const fs::path PROGS_PATH = BASE_PATH / "vm/programs/";

    const std::vector<std::pair<std::string, std::string>> incs = {
        { "mmio.inc", load(INCS_PATH, "mmio.inc") },
        { "layout.inc", load(INCS_PATH, "layout.inc") },
        { "syscalls.inc", load(INCS_PATH, "syscalls.inc") }
    };
    auto pool = [&](std::vector<std::pair<std::string, std::string>> v) {
        v.insert(v.end(), incs.begin(), incs.end()); return v;
    };
    const std::vector<std::pair<std::string, std::string>> kernel_soures = pool({
        { "kernel.asm", load(KERNEL_PATH, "kernel.asm") },
        { "storage.asm", load(KERNEL_PATH, "storage.asm") },
        { "loader.asm", load(KERNEL_PATH, "loader.asm") } }
        );
    const std::vector<ProgramSource> programs = {
    /* 0 */ { "main", pool({ { "main.asm", load(PROGS_PATH, "main.asm") } }) },
    /* 1 */ { "mem_fault", pool({ { "mem_fault.asm", load(PROGS_PATH, "mem_fault.asm") } }) },
    /* 2 */ { "stack_overflow.asm", pool({ { "stack_overflow.asm", load(PROGS_PATH, "stack_overflow.asm") } }) },
    /* 3 */ { "spawndemo", pool({ { "spawndemo.asm", load(PROGS_PATH, "spawndemo.asm") } }) },
    /* 4 */ { "child", pool({ { "child.asm", load(PROGS_PATH, "child.asm") } }) },
    /* 5 */ { "proctest", pool({ { "proctest.asm", load(PROGS_PATH, "proctest.asm") } }) },
    /* 6 */ { "pt_exit7", pool({ { "pt_exit7.asm", load(PROGS_PATH, "pt_exit7.asm") } }) },
    /* 7 */ { "pt_fault", pool({ { "pt_fault.asm", load(PROGS_PATH, "pt_fault.asm") } }) },
    /* 8 */ { "pt_yield", pool({ { "pt_yield.asm", load(PROGS_PATH, "pt_yield.asm") } }) },
    /* 9 */ { "pt_regs", pool({ { "pt_regs.asm", load(PROGS_PATH, "pt_regs.asm") } }) },
    /* 10 */ { "pt_grand", pool({ { "pt_grand.asm", load(PROGS_PATH, "pt_grand.asm") } }) },
    /* 11 */ { "pt_orphan", pool({ { "pt_orphan.asm", load(PROGS_PATH, "pt_orphan.asm") } }) }
    };

    std::cout << "ROM_BUILD: " << env.build_rom(pool({ { "boot.asm", load(BOOT_PATH, "boot.asm") } })) << std::endl;
    std::cout << "KERNEL_BUILD: " << env.build_disk(kernel_soures, programs) << std::endl;

    env.mb.reset();

    const auto start = std::chrono::high_resolution_clock::now();
    const RunResult rr = env.start();
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec (1e-6 s)" << std::endl;
    std::cout << "RunResult: " << to_str(rr) << std::endl;
    std::cout << "EXIT CODE: " << env.exit_code << std::endl;

    std::cout << "\n---------- FINAL CPU STATE ----------\n";
    std::vector<std::pair<std::string, uint8_t>> local_reg_table = {
        { "r0", 0 },
        { "r1", 1 },
        { "r2", 2 },
        { "r3", 3 },
        { "r4", 4 },
        { "r5", 5 },
        { "r6", 6 },
        { "r7", 7 },
        { "r8", 8 },
        { "r9", 9 },
        { "r10", 10 },
        { "trp", 11 },
        { "cmp", 12 },
        { "fp", 13 },
        { "sp", 14 },
        { "pc", 15 },
    };
    for (const auto& [reg_name, idx] : local_reg_table)
        std::cout << reg_name << " = " << static_cast<int>(env.mb.cpu->core.regs[idx]) << "\n";

    std::cout << "\n---------- DEBUG ? ----------\n";

    env.build_rom(pool({ { "boot.asm", load(BOOT_PATH, "boot.asm") } }));
    env.build_disk(kernel_soures, programs);
    env.mb.reset();

    env.start_debug();

    std::cout << "KEYS: \n";
    std::cout << " - r : to see regs \n";
    std::cout << " - f : to see fregs \n";
    std::cout << " - i : to see PC, SP, FP \n";
    std::cout << " - e : to exit \n";
    std::cout << "\n";

    while (true) {
        std::string in;
        std::cin >> in;

        StepInfo SI = env.step();
        std::cout << "PC = " << SI.PC;
        // 17 spaces max
        for (size_t i = 5 + std::to_string(SI.PC).size(); i < 17; i++)
            std::cout << ' ';
        std::cout << " --> " << SI.line << "\n";
        if (in == "f")
            for (const auto& [reg_name, idx] : local_reg_table)
                if (reg_name.starts_with("f"))
                    std::cout << reg_name << " = " << static_cast<int>(SI.fregs[idx]) << "\n";
        if (in == "r")
            for (const auto& [reg_name, idx] : local_reg_table)
                if (!reg_name.starts_with("f"))
                    std::cout << reg_name << " = " << static_cast<int>(SI.regs[idx]) << "\n";
        if (in == "i") {
            std::cout << "PC = " << SI.PC << "\n";
            std::cout << "SP = " << SI.SP << "\n";
            std::cout << "FP = " << SI.FP << "\n";
        }
        if (in == "e")
            break;
    }


    return 0;
}