#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include "mother_board.h"
#include "ASM/asm_interpreter.h"

enum class RunMode {
    AUTO,
    STEP
};

template <size_t RAM_SIZE, size_t PROGRAM_SIZE> //65 5535 lines, 64Kb
struct EnvironmentManager {
    std::unique_ptr<MotherBoard<RAM_SIZE, PROGRAM_SIZE>> mb;
    AsmInterpreter<RAM_SIZE, PROGRAM_SIZE> interpreter;
    RunMode mode = RunMode::STEP;
    std::string error_msg;

    EnvironmentManager() {
        mb = std::make_unique<MotherBoard<RAM_SIZE, PROGRAM_SIZE>>();
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

        mb->stop();
        mb->reset();

        for (auto& [name, var] : interpreter.vars) {
            for (size_t i = 0; i < var.init.size(); ++i)
                mb->ram[var.addr + i] = var.init[i];
        }

        mb->load_prog(interpreter.program);
    }

    int get_from_ram(size_t addr) {
        if (addr < mb->ram.size())
            return mb->ram[addr];
        return 0;
    }

    int get_from_reg(size_t reg) {
        if (reg < mb->cpu.core.regs.size())
            return mb->cpu.core.regs[reg];
        return 0;
    }

    Flags get_cpu_flags() {
        return mb->cpu.core.flags;
    }

    void set_mod(RunMode new_mode) {
        mode = new_mode;
    }

    void start() {
        mb->start();

        if (mode == RunMode::AUTO)
            while (mb->running) {
                if (mb->cpu.core.PC < mb->rom.size())
                    mb->running = mb->cpu.core.step(mb->rom[mb->cpu.core.PC], mb->ram);
                else
                    mb->running = false;
            }
    }

    void step(){
        if (!mb->running) return;

        if (mb->cpu.core.PC < mb->rom.size())
            mb->running = mb->cpu.core.step(mb->rom[mb->cpu.core.PC], mb->ram);
        else
            mb->running = false;
    }

    std::string get_error_msg() {
        return error_msg;
    }

};


#endif