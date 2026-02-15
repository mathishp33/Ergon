#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

//IRL bytecode program is ROM, EEPROM or Flash. -> it contains the firmware (BIOS, bootloader, ...)

#include "cpu.h"

#include <atomic>
#include <array>

/*
 ----------------- VM SCHEMATIC -----------------
 ┌──────────CPU──────────┐
 │                       │    ┌───┐
 │                 ┌───┐ ◄────►RAM│
 │                 │ALU│ │    └───┘
 │                 └─▲─┘ │
 │                   │   │
 │┌───────────┐   ┌──▼─┐ │    ┌───┐
 ││ registers │◄──┤Core│ │◄───┤ROM│
 │└───────────┘   └────┘ │    └───┘
 └───────────────────────┘
 */

//I NEED TO IMPLEMENT malloc and free alike instruction !

template <size_t REG_COUNT, size_t MEM_SIZE, size_t PROGRAM_SIZE>
struct MotherBoard {
    SimpleCPU<REG_COUNT, MEM_SIZE> cpu;
    std::array<uint8_t, MEM_SIZE> ram{};
    std::array<uint32_t, PROGRAM_SIZE> rom{};
    std::atomic<bool> running;

    void reset() {
        running = false;

        ram.fill(0);
        rom.fill(0);
    }

    void load(const std::array<uint32_t, PROGRAM_SIZE>& program) {
        rom = program;
    }

    void start() {
        running = true;

        while (running) {
            running = cpu.core.step(rom[cpu.core.PC], ram);
        }
    }
};


#endif