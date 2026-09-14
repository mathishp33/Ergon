#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"
#include "asm/data.h"

#include <atomic>
#include <vector>


struct MotherBoard {
    std::shared_ptr<SimpleCPU> cpu;
    std::vector<uint8_t> ram{ };
    std::vector<DecodedInstr> rom{ };

    MotherBoard(size_t RAM_SIZE) {
        if (RAM_SIZE >= 0xFFFFFF - 1) RAM_SIZE = 0xFFFFFF - 1;
        ram.resize(RAM_SIZE);
        std::ranges::fill(ram, 0);
        cpu = std::make_shared<SimpleCPU>(ram);
    }

    void reset() {
        std::ranges::fill(ram, 0);
        std::ranges::fill(rom, DecodedInstr());
    }

    void load_prog(const std::vector<DecodedInstr>& program) {
        rom = program;
    }
};


#endif