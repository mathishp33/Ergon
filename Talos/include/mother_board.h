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
    std::array<uint8_t, RAM_SIZE> ram{};
    std::vector<uint32_t> rom{};
    std::atomic<bool> running;

    void reset() {
        running = false;

        ram.fill(0);
        for (auto& elem : rom) elem = 0;
    }

    void load_prog(const std::vector<uint32_t>& program) {
        rom = program;
    }

    void start() {
        running = true;
    }

    void stop() {
        running = false;
    }
};


#endif