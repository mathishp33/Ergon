#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include "computer/mother_board.h"
#include "computer/op_handler.h"
#include "asm/interpreter.h"

enum class RunMode {
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
    AsmInterpreter<PROGRAM_SIZE> interpreter;
    RunMode mode = RunMode::STEP;
    std::string error_msg;
    std::unique_ptr<StepInfo> step_info = nullptr;

    EnvironmentManager() {
        mb.cpu.core = std::make_unique<SimpleCore>(mb.ram);
    }

    void build(std::string input_program) {
        error_msg = "";
        ErrorInfo error_info = interpreter.decode(input_program);
        if (error_info.error_code != ErrorCode::OK) {
            error_msg = ErrorCode_to_String(error_info)  + "\n";
            error_msg += interpreter.lines[error_info.index_line];
            error_msg += "\n";
            for (size_t i = 0; i < interpreter.lines[error_info.index_line].size(); i++)
                error_msg += "^";
            error_msg += "\n";
        }

        mb.reset();

        for (size_t i = 0; i < interpreter.data.size(); i++) {
            mb.ram[i] = interpreter.data[i];
        }

        mb.load_prog(interpreter.text);
    }

    int get_from_ram(size_t addr) {
        if (addr < mb.ram.size())
            return mb.ram[addr];
        return 0;
    }

    int get_from_reg(size_t reg) {
        if (reg < mb.cpu.core.regs.size())
            return mb.cpu.core.regs[reg];
        return 0;
    }

    Flags get_cpu_flags() {
        return mb.cpu.core.flags;
    }

    void set_mod(RunMode new_mode) {
        mode = new_mode;
    }

    void start() {
        if (mode == RunMode::AUTO)
            while (mb.cpu.core->PC < mb.rom.size()) {
                step_instr(*mb.cpu.core, mb.rom[mb.cpu.core->PC]);
            }
    }

    StepInfo step(){
        if (mb.cpu.core->PC < mb.rom.size())
            step_instr(mb.cpu.core, mb.rom[mb.cpu.core->PC]);

        return StepInfo(mb.cpu.core->regs, mb.rom[mb.cpu.core->PC]);
    }

    std::string get_error_msg() {
        return error_msg;
    }

};


#endif