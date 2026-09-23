#ifndef ERGON_CPU_H
#define ERGON_CPU_H

#include <memory>

#include "core.h"

struct SimpleCPU {
    SimpleCore core;

    SimpleCPU(SystemBus& bus, uint32_t ram_size) : core(bus, ram_size) {};

};


#endif