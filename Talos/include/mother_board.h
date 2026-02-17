#ifndef ERGON_MOTHER_BOARD_H
#define ERGON_MOTHER_BOARD_H

#include "cpu.h"

#include <atomic>
#include <array>
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

template <size_t RAM_SIZE = 0x10000, size_t PROGRAM_SIZE = 0xFFFF>
struct MotherBoard {
    SimpleCPU<RAM_SIZE> cpu;
    std::vector<uint8_t> ram{};
    std::vector<uint32_t> rom{};
    std::atomic<bool> running;

    MotherBoard() {
        ram.resize(RAM_SIZE);
        for (auto& elem : ram) elem = 0;
    }

    void reset() {
        running = false;

        for (auto& elem : ram) elem = 0;
        for (auto& elem : rom) elem = 0;
    }

    void load_prog(const std::vector<uint32_t>& program) {
        rom = program;
        if (rom.size() > PROGRAM_SIZE)
            rom.resize(PROGRAM_SIZE);
        if (rom.size() < PROGRAM_SIZE) {
            for (size_t i = rom.size(); i < PROGRAM_SIZE; i++) {
                rom.push_back(0x00000000);
            }
        }
    }

    void start() {
        running = true;
    }

    void stop() {
        running = false;
    }
};


#endif