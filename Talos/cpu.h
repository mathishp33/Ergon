#ifndef ERGON_CPU_H
#define ERGON_CPU_H

#include "core.h"

template <size_t REG_COUNT, size_t MEM_SIZE>
struct SimpleCPU {
    SimpleCore<REG_COUNT, MEM_SIZE> core;


};


#endif