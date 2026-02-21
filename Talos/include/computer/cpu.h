#ifndef ERGON_CPU_H
#define ERGON_CPU_H

#include <memory>

#include "core.h"

struct SimpleCPU {
    SimpleCore core;

    SimpleCPU(std::vector<uint8_t>& ram) : core(ram) {};

};


#endif