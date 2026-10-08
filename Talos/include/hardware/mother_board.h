#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"
#include "bus.h"
#include "software/data.h"

#include <vector>


struct MotherBoard {
    std::vector<uint8_t> hard_drive;
    std::vector<uint8_t> ram;
    std::vector<uint8_t> rom;
    uint32_t rom_entry_pc = ROM_BASE;
    MMIO mmio;
    SystemBus bus;
    std::shared_ptr<SimpleCPU> cpu;

    MotherBoard(size_t ram_size, const uint32_t requested) : mmio(hard_drive, ram), bus(ram, rom, mmio) {
        if (ram_size >= 0xFFFFFF - 1) ram_size = 0xFFFFFF - 1;
        ram.resize(ram_size);
        std::ranges::fill(ram, 0);

        cpu = std::make_shared<SimpleCPU>(bus, static_cast<uint32_t>(ram.size()));
        set_stack_size(requested);
    }

    void load_rom(const std::vector<uint8_t>& bytes, const uint32_t entry_pc = ROM_BASE) {
        rom = bytes;
        rom_entry_pc = entry_pc;
    }

    void set_stack_size(const uint32_t new_stack_size) const {
        if (!new_stack_size)
            throw std::invalid_argument("in set_stack_size(): stack size must be greater than 0.");
        if (new_stack_size >= static_cast<uint32_t>(ram.size()))
            throw std::invalid_argument("in set_stack_size(): stack size must be lesser than ram.size()");
        cpu->core.stack_limit = static_cast<uint32_t>(ram.size()) - new_stack_size;
    }

    void reset() {
        std::ranges::fill(ram, 0);
        cpu->core.reset(static_cast<uint32_t>(ram.size()));
        cpu->core.PC = rom_entry_pc;
    }

    void reset_hard_drive() {
        std::ranges::fill(hard_drive, 0);
    }
};


#endif