#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include "computer/mother_board.h"
#include "computer/instructions_handler/step_handler.h"
#include "computer/instructions_handler/run_handler.h"
#include "asm/decoder.h"
#include "asm/linker.h"

struct StepInfo {
    std::array<uint32_t, 16> regs{};
    std::array<uint32_t, 16> fregs{};
    uint32_t& PC = regs[16 - 1];
    uint32_t& SP = regs[16 - 2];

    DecodedInstr instr;

    StepInfo() = default;
    StepInfo(const MotherBoard& mb) {
        regs = mb.cpu.core.regs;
        fregs = mb.cpu.core.fregs;
        PC = mb.cpu.core.PC;
        SP = mb.cpu.core.SP;

        instr = mb.rom[mb.cpu.core.PC];
    }
};


struct EnvironmentManager {
    size_t RAM_SIZE = 65535; // 2^16 - 1
    MotherBoard mb;
    AsmDecoder decoder;

    EnvironmentManager(size_t RAM_SIZE = 65535) : RAM_SIZE(RAM_SIZE), mb(MotherBoard(RAM_SIZE)) {}

    std::string handle_error(const std::string& file_name, ErrorInfo e) {
        std::string e_msg = "Error at line " + std::to_string(e.index_line) + " in file " + file_name + ": \n";
        e_msg += e.message + "\n";
        e_msg += "\n";
        e_msg += decoder.lines[e.index_line];
        e_msg += "\n";
        for (size_t i = 0; i < decoder.lines[e.index_line].size(); i++)
            e_msg += "^";
        e_msg += "\n";
        return e_msg;
    }

    ErrorCode load_ram( const std::vector<uint8_t>& data, const std::vector<uint8_t>& rodata) {
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[i] = data[i];
        }
        for (size_t i = 0; i < rodata.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[i] = rodata[i];
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
        if (load_ram(linked_bin.data, linked_bin.rodata) != ErrorCode::OK) return handle_error("linked binary", ErrorInfo(ErrorCode::RAM_OVERFLOW, 0));

        mb.cpu.core.PC = linked_bin.entry_pc;
        mb.load_prog(linked_bin.text);
        return "";
    }

    int get_from_ram(size_t addr) {
        if (addr < mb.ram.size())
            return mb.ram[addr];
        return 0;
    }

    uint32_t get_from_reg(size_t reg_index) {
        if (reg_index < mb.cpu.core.regs.size())
            return mb.cpu.core.regs[reg_index];
        return 0;
    }
    uint32_t get_from_reg(const std::string& reg_name) {
        auto [e, reg_index] = parse_reg(reg_name);
        if (e.code != ErrorCode::OK) return 0;

        if (reg_index < mb.cpu.core.regs.size())
            return mb.cpu.core.regs[reg_index];
        return 0;
    }

    void start() {
        run(mb.cpu.core, mb.rom);
    }

    StepInfo step() {
        if (mb.cpu.core.PC >= mb.rom.size()) return { };
        step_instr(mb.cpu.core, mb.rom[mb.cpu.core.PC]);

        return { mb };
    }
};


#endif