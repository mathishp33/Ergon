#ifndef ERGON_MEMORY_H
#define ERGON_MEMORY_H

#include <cstdint>
#include <vector>


struct Bus {
    bool used = false; //cannot write when memory is being written
};


struct RAMBus : Bus {
    const uint32_t max_addr_size = 0xFFFFFFFF - 1;
    std::vector<uint8_t>& ram;

    RAMBus(std::vector<uint8_t>& ram) : ram(ram) {}

    uint32_t load32(uint32_t addr) {
        if (addr + 3 >= ram.size()) return 0;
        return ram[addr] | (ram[addr + 1] << 8) | (ram[addr + 2] << 16) | (ram[addr + 3] << 24);
    }
    uint16_t load16(uint32_t addr) {
        if (addr + 1 >= ram.size()) return 0;
        return ram[addr] |
               (ram[addr + 1] << 8);
    }
    uint8_t load8(uint32_t addr) {
        if (addr >= ram.size()) return 0;
        return ram[addr];
    }
    // addr -> ram
    void store32(uint32_t addr, uint32_t value) {
        if (addr + 3 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
        ram[addr + 2] = (value >> 16) & 0xFF;
        ram[addr + 3] = (value >> 24) & 0xFF;
    }
    void store16(uint32_t addr, uint16_t value) {
        if (addr + 1 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
    }
    void store8(uint32_t addr, uint8_t value) const
    {
        if (addr >= ram.size()) return;
        ram[addr] = value;
    }
};

struct Memory {
    std::vector<uint8_t> memory;
    Bus bus;
};

struct RAM : Memory {
    void reset() {
        std::fill(memory.begin(), memory.end(), 0);
    }
};

struct ROM : Memory {
    void initialize(const std::vector<uint8_t>& new_memory) {
        memory = new_memory;
    }
};

#endif