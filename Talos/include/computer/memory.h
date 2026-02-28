#ifndef ERGON_MEMORY_H
#define ERGON_MEMORY_H

#include <cstdint>
#include <vector>


struct Bus {
    bool used = false; //cannot write when memory is being written
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