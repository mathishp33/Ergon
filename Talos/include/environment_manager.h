#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include <chrono>
#include <thread>

#include "computer/mother_board.h"
#include "computer/instructions_handler/step_handler.h"
#include "computer/instructions_handler/run_handler.h"
#include "asm/assembler.h"
#include "asm/linker.h"

struct StepInfo {
    std::array<uint32_t, 16> regs{};
    std::array<uint32_t, 16> fregs{};
    uint32_t& PC = regs[16 - 1];
    uint32_t& SP = regs[16 - 2];

    DecodedInstr instr;

    StepInfo() = default;
    StepInfo(const MotherBoard& mb) {
        regs = mb.cpu->core.regs;
        fregs = mb.cpu->core.fregs;
        PC = mb.cpu->core.PC;
        SP = mb.cpu->core.SP;

        instr = mb.rom[mb.cpu->core.PC];
    }
};


struct EnvironmentManager {
    size_t RAM_SIZE = 65535; // 2^16 - 1
    MotherBoard mb;
    Assembler decoder;
    int exit_code = 1;
    std::atomic<bool> running = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;

    EnvironmentManager(size_t ram_size = 65535) :
     RAM_SIZE(ram_size), mb(MotherBoard(ram_size)) {}

    std::string handle_error(const std::string& file_name, const ErrorInfo& e) {
        std::string e_msg = "Error at line " + std::to_string(e.index_line) + " in file " + file_name + ": \n";
        e_msg += e.message + "\n";
        e_msg += "\n";
        if (decoder.lines.size() > e.index_line) {
            e_msg += decoder.lines[e.index_line];
            e_msg += "\n";
            for (size_t i = 0; i < decoder.lines[e.index_line].size(); i++)
                e_msg += "^";
            e_msg += "\n";
        }
        return e_msg;
    }

    ErrorCode load_ram( const std::vector<uint8_t>& data, const std::vector<uint8_t>& rodata, uint32_t bss_size) {
        if (data.size() + rodata.size() + bss_size > mb.cpu->core.stack_limit)
            return ErrorCode::RAM_OVERFLOW;

        for (size_t i = 0; i < data.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[i] = data[i];
        }
        for (size_t i = 0; i < rodata.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[data.size() + i] = rodata[i];
        }
        size_t bss_start = data.size() + rodata.size();
        for (size_t i = 0; i < bss_size; i++) {
            if (bss_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[bss_start + i] = 0;
        }
        return ErrorCode::OK;
    }

    std::string build_single(const std::string& program) {
        return build({ { "main" , program } });
    }

    //args are { { <name/path>, <my_program> } }, returns error message
    std::string build(const std::vector<std::pair<std::string, std::string>>& inputs) {
        std::vector<ObjectFile> obj_files;
        std::vector<std::string> error_infos;
        for (const auto& [name, program] : inputs) {
            auto [obj_file, error_info] = decoder.decode(program);
            if (error_info.code != ErrorCode::OK) return handle_error(name, error_info);
            obj_files.emplace_back(obj_file);
        }
        auto [e, linked_bin] = link(obj_files);
        if (e.code != ErrorCode::OK) return handle_error("linked binary", e);

        mb.reset();
        mb.set_stack_size(linked_bin.stack_size);
        if (load_ram(linked_bin.data, linked_bin.rodata, linked_bin.bss_size) != ErrorCode::OK) return handle_error("linked binary", ErrorInfo(ErrorCode::RAM_OVERFLOW, 0));

        mb.cpu->core.PC = linked_bin.entry_pc;
        mb.load_prog(linked_bin.text);
        return "";
    }

    int get_from_ram(size_t addr) {
        if (addr < mb.ram.size())
            return mb.ram[addr];
        return 0;
    }

    uint32_t get_from_reg(const std::string& reg_name) {
        auto [e, reg_index] = parse_reg(reg_name);
        if (e.code != ErrorCode::OK) return 0;

        if (reg_index < mb.cpu->core.regs.size()) {
            if (reg_name[0] == 'f') return mb.cpu->core.fregs[reg_index];
            return mb.cpu->core.regs[reg_index];
        }
        return 0;
    }

    void start() {
        running = true;
        exit_code = 1;
        start_time = std::chrono::system_clock::now();
        run(mb.cpu->core, mb.rom, [this]() { handle_syscall(); return exit_code; });
        running = false;
    }

    StepInfo step() {
        if (mb.cpu->core.PC == 0) {
            running = true;
            exit_code = 1;
            start_time = std::chrono::system_clock::now();
        }
        if (mb.cpu->core.PC >= mb.rom.size()) return { };
        step_instr(mb.cpu->core, mb.rom[mb.cpu->core.PC], [this]() { handle_syscall(); return exit_code; });

        return { mb };
    }

    void handle_syscall() {
        switch (mb.cpu->core.regs[0]) {
        case ABI::EXIT:
            sys_exit();
            break;
        case ABI::WRITE:
            sys_write();
            break;
        case ABI::READ:
            sys_read();
            break;
        case ABI::CLOCK:
            sys_clock();
            break;
        case ABI::TIME:
            sys_time();
            break;
        case ABI::DISK_READ:
            break;
        case ABI::DISK_WRITE:
            break;
        default:
            break;
        }
    }

    void sys_exit() {
        running = false;
        exit_code = 0;
    }

    void sys_write() {
        uint32_t addr = mb.cpu->core.regs[1];
        uint32_t size = mb.cpu->core.regs[2];

        if (addr > mb.ram.size() || size > mb.ram.size() - addr) {
            exit_code = -1;
            mb.cpu->core.regs[0] = static_cast<uint32_t>(-1);
            return;
        }
        uint8_t* data = &mb.ram[addr];

        std::cout.write(reinterpret_cast<const char*>(data), size);
    }

    void sys_read() {
        uint32_t addr = mb.cpu->core.regs[1];
        uint32_t size = mb.cpu->core.regs[2];

        if (addr > mb.ram.size() || size > mb.ram.size() - addr) {
            mb.cpu->core.regs[0] = static_cast<uint32_t>(-1);
            exit_code = -1;
            return;
        }
        char* data = reinterpret_cast<char*>(&mb.ram[addr]);

        std::streamsize n = 0;
        std::cin.read(data, size);
        n = std::cin.gcount();
        if (std::cin.eof()) std::cin.clear();

        mb.cpu->core.regs[0] = static_cast<uint32_t>(n);
    }

    void sys_clock() {
        const auto now = std::chrono::system_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

        mb.cpu->core.regs[0] = static_cast<uint32_t>(elapsed);
    }

    void sys_time() {
        const auto now = std::chrono::system_clock::now();
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        mb.cpu->core.regs[0] = static_cast<uint32_t>(timestamp);
    }
};


#endif