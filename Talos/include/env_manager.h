#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include "computer/mother_board.h"
#include "computer/instructions_handler/step_handler.h"
#include "computer/instructions_handler/run_handler.h"
#include "asm/decoder.h"
#include "asm/linker.h"

enum class ExecMode {
    AUTO,
    STEP
};

struct StepInfo {
    std::array<uint32_t, 16> regs{};
    uint32_t& PC = regs[16 - 1];
    uint32_t& SP = regs[16 - 2];

    uint8_t opcode = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;

    bool running = false;

    StepInfo() = default;
    StepInfo(const std::array<uint32_t, 16>& regs, const uint32_t instr, bool running) : regs(regs), running(running) {
        opcode = instr & 0xFF;
        rd = (instr >> 8) & 0xFF;
        rs1 = (instr >> 16) & 0xFF;
        rs2 = (instr >> 24) & 0xFF;
    }
};


template <size_t PROGRAM_SIZE> //65 5535 lines
struct EnvironmentManager {
    MotherBoard<PROGRAM_SIZE> mb;
    AsmDecoder<PROGRAM_SIZE> decoder;
    ExecMode mode = ExecMode::STEP;

    EnvironmentManager() = default;

    std::string handle_error(const std::string& file_name, ErrorInfo e_info) {
        std::string e_msg = "Error in " + file_name + ": \n";
        e_msg += ErrorCode_to_String(e_info) + "\n";
        e_msg += decoder.lines[e_info.index_line];
        e_msg += "\n";
        for (size_t i = 0; i < decoder.lines[e_info.index_line].size(); i++)
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

    //returns error message
    std::string build_single(std::string input_program) {
        std::string file_name = "main";
        auto [obj_file, error_info] = decoder.decode(input_program);
        if (error_info.error_code != ErrorCode::OK) return handle_error(file_name, error_info);

        mb.reset();
        if (load_ram(obj_file.data, obj_file.rodata) != ErrorCode::OK) return handle_error(file_name, ErrorInfo(ErrorCode::RAM_OVERFLOW, 0));

        mb.load_prog(obj_file.text);
        return "";
    }

    std::string build_lib(const std::vector<std::string>& input_programs) {
        std::vector<ObjectFile> obj_files;
        std::vector<std::string> error_infos;
        for (size_t i = 0; i < input_programs.size(); i++) {
            auto [obj_file, error_info] = decoder.decode(input_programs[i]);
            if (error_info.error_code != ErrorCode::OK) return handle_error("file " + std::to_string(i), error_info);
            obj_files.emplace_back(obj_file);
        }

        auto [linked_bin, e_code] = link(obj_files);

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

    int get_from_reg(size_t reg_index) {
        if (reg_index < mb.cpu.core.regs.size())
            return mb.cpu.core.regs[reg_index];
        return 0;
    }
    int get_from_reg(const std::string& reg_name) {
        auto [e_code, reg_index] = parse_reg(reg_name);
        if (e_code != ErrorCode::OK) return 0;

        if (reg_index < mb.cpu.core.regs.size())
            return mb.cpu.core.regs[reg_index];
        return 0;
    }

    Flags get_cpu_flags() {
        return mb.cpu.core.flags;
    }

    void set_mod(ExecMode new_mode) {
        mode = new_mode;
    }

    void start() {
        if (mode == ExecMode::AUTO) run(mb.cpu.core, mb.rom);

    }

    StepInfo step() {
        if (mb.cpu.core.PC < mb.rom.size()) step_instr(mb.cpu.core, mb.rom[mb.cpu.core.PC]);

        return StepInfo(mb.cpu.core.regs, mb.rom[mb.cpu.core.PC]);
    }
};


#endif