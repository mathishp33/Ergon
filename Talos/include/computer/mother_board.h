#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"
#include "asm/data.h"

#include <atomic>
#include <vector>


struct MotherBoard {
    SimpleCPU cpu;
    std::vector<uint8_t> ram{};
    std::vector<DecodedInstr> rom{};
    bool running = false;

    MotherBoard(size_t RAM_SIZE) : cpu(ram) {
        if (RAM_SIZE >= 0xFFFFFF - 1) RAM_SIZE = 0xFFFFFF - 1;
        ram.resize(RAM_SIZE);
        std::ranges::fill(ram, 0);
    }

    void reset() {
        running = false;

        std::ranges::fill(ram, 0);
        std::ranges::fill(rom, DecodedInstr());
    }

    void load_prog(const std::vector<DecodedInstr>& program, size_t max_size = 0xFFFFFF - 1) {
        if (max_size >= 0xFFFFFF - 1) max_size = 0xFFFFFF - 1;
        rom = program;
        if (rom.size() > max_size) rom.resize(max_size);
    }
};


#endif