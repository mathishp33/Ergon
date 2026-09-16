#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"
#include "asm/data.h"

#include <atomic>
#include <vector>


struct MotherBoard {
    std::shared_ptr<SimpleCPU> cpu;
    std::vector<uint8_t> ram;
    std::vector<DecodedInstr> rom;
    std::vector<uint8_t> hard_drive;

    MotherBoard(size_t ram_size) {
        if (ram_size >= 0xFFFFFF - 1) ram_size = 0xFFFFFF - 1;
        ram.resize(ram_size);
        std::ranges::fill(ram, 0);

        cpu = std::make_shared<SimpleCPU>(ram);
        set_stack_size(0);
    }

    void set_stack_size(uint32_t requested) {
        uint32_t stack_size = (requested == 0) ? 4096 : requested;
        uint32_t max_stack = static_cast<uint32_t>(ram.size()) / 4;
        stack_size = std::clamp((int)stack_size, 256, std::max(256, (int)max_stack));
        cpu->core.stack_limit = static_cast<uint32_t>(ram.size()) - stack_size;
    }

    void reset() {
        std::ranges::fill(ram, 0);
        std::ranges::fill(rom, DecodedInstr());
        cpu->core.reset();
    }

    void load_prog(const std::vector<DecodedInstr>& program) {
        rom = program;
    }

    void reset_hard_drive() {
        std::ranges::fill(hard_drive, 0);
    }
};


#endif