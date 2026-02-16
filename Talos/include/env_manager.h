#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include "mother_board.h"
#include "asm_interpreter.h"

enum class RunMode {
    AUTO,
    STEP
};

template <size_t RAM_SIZE, size_t PROGRAM_SIZE> //65 5535 lines, 64Kb
struct EnvironmentManager {
    std::unique_ptr<MotherBoard<RAM_SIZE, PROGRAM_SIZE>> motherboard;
    AsmInterpreter<PROGRAM_SIZE> interpreter;
    RunMode mode = RunMode::STEP;
    std::string error_msg;

    EnvironmentManager() {
        motherboard = std::make_unique<MotherBoard<RAM_SIZE, PROGRAM_SIZE>>();
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


        motherboard->stop();
        motherboard->reset();

        interpreter.program.resize(PROGRAM_SIZE);
        motherboard->load_prog(interpreter.program);
    }

    uint8_t get_from_ram(size_t addr) {
        if (addr > motherboard->ram.size())
            return motherboard->ram[addr];
        return 0;
    }

    void start(RunMode new_mode) {
        mode = new_mode;
        motherboard.start();
        if (mode == RunMode::AUTO)
            motherboard.run();
    }

    std::string get_error_msg() {
        return error_msg;
    }

};


#endif