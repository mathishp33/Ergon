#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"

#include <atomic>
#include <vector>

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

template <size_t PROGRAM_SIZE = 0xFFFF>
struct MotherBoard {
    SimpleCPU cpu;
    std::vector<uint8_t> ram{};
    std::vector<DecodedInstr> rom{};
    std::atomic<bool> running;

    size_t RAM_SIZE = 65536; //2^16 bits addresses

    MotherBoard() {
        ram.resize(RAM_SIZE);
        for (auto& elem : ram) elem = 0;
    }

    void reset() {
        running = false;

        for (auto& elem : ram) elem = 0;
        for (auto& elem : rom) elem = DecodedInstr();
    }

    void load_prog(const std::vector<DecodedInstr>& program) {
        rom = program;
        if (rom.size() > PROGRAM_SIZE)
            rom.resize(PROGRAM_SIZE);
        if (rom.size() < PROGRAM_SIZE) {
            for (size_t i = rom.size(); i < PROGRAM_SIZE; i++) {
                rom.emplace_back();
            }
        }
    }
};


#endif