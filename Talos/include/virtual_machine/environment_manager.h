#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include <chrono>
#include <fstream>
#include <thread>

#include "../hardware/mother_board.h"
#include "../hardware/instructions_handler/step_handler.h"
#include "../hardware/instructions_handler/run_handler.h"
#include "../software/assembler.h"
#include "../software/linker.h"

struct StepInfo {
    std::array<uint32_t, 16> regs;
    std::array<uint32_t, 16> fregs;
    uint32_t& PC = regs[15];
    uint32_t& SP = regs[14];
    uint32_t& FP = regs[13];

    DecodedInstr instr;
    PrivMode mode = PrivMode::KERNEL;
    uint32_t trap_vector = 0;

    StepInfo() = default;
    StepInfo(const MotherBoard& mb) {
        regs = mb.cpu->core.regs;
        fregs = mb.cpu->core.fregs;
        PC = mb.cpu->core.PC;
        SP = mb.cpu->core.SP;
        FP = mb.cpu->core.FP;
        mode = mb.cpu->core.mode;
        trap_vector = mb.cpu->core.trap_vector;

        instr = mb.cpu->core.bus.fetch_instr(mb.cpu->core.PC);
    }
};


struct EnvironmentManager {
    size_t RAM_SIZE = 65535; // 2^16 - 1
    MotherBoard mb;
    Assembler decoder;
    int exit_code = 1;
    std::atomic<bool> running = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;

    EnvironmentManager(size_t ram_size = 65535, const std::string& path_to_hard_drive = "") :
     RAM_SIZE(ram_size), mb(MotherBoard(ram_size)) {
        if (!path_to_hard_drive.empty()) {
            std::ifstream file(path_to_hard_drive, std::ios::binary);

            if (!file)
                throw std::runtime_error("Cannot open file: " + path_to_hard_drive);

            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            mb.hard_drive.resize(size);

            if (!file.read(reinterpret_cast<char*>(mb.hard_drive.data()), size))
                throw std::runtime_error("Cannot read file: " + path_to_hard_drive);
        }
    }

    std::string handle_error(const std::string& file_name, const ErrorInfo& e) {
        std::string e_msg = "Error " + std::to_string((int)e.code) + " at line " + std::to_string(e.index_line) +
            " in file " + file_name + ": \n";
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

    // memory: [ text ][ data ][ rodata ][ bss ][ stack ]
    ErrorCode load_ram(const std::vector<uint8_t>& text, const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& rodata, uint32_t bss_size) {
        if ((uint64_t)text.size() + data.size() + rodata.size() + bss_size > mb.cpu->core.stack_limit)
            return ErrorCode::RAM_OVERFLOW;

        for (size_t i = 0; i < text.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[i] = text[i];
        }
        size_t data_start = text.size();
        for (size_t i = 0; i < data.size(); i++) {
            if (data_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[data_start + i] = data[i];
        }
        size_t rodata_start = data_start + data.size();
        for (size_t i = 0; i < rodata.size(); i++) {
            if (rodata_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[rodata_start + i] = rodata[i];
        }
        size_t bss_start = rodata_start + rodata.size();
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

        const std::vector<uint8_t> text_bytes = to_bytes(linked_bin.text);
        if (load_ram(text_bytes, linked_bin.data, linked_bin.rodata, linked_bin.bss_size) != ErrorCode::OK)
            return handle_error("linked binary", ErrorInfo(ErrorCode::RAM_OVERFLOW, 0));

        mb.cpu->core.PC = linked_bin.entry_pc; // déjà une adresse en octets (voir linker.h)
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
        run(mb.cpu->core);
        //le kernel place le code de sortie dans r0 avant HALT.
        exit_code = static_cast<int>(mb.cpu->core.regs[0]);
        running = false;
    }

    // StepInfo step() {
    //     if (mb.cpu->core.PC == 0) {
    //         running = true;
    //         exit_code = 1;
    //         start_time = std::chrono::system_clock::now();
    //     }
    //     if (mb.cpu->core.PC >= mb.ram.size()) return { };
    //     const DecodedInstr current = mb.cpu->core.bus.fetch_instr(mb.cpu->core.PC);
    //     step_instr(mb.cpu->core, current, [this]() { handle_syscall(); return exit_code; });
    //
    //     return { mb };
    // }

};


#endif