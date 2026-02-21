#ifndef ERGON_CPU_H
#define ERGON_CPU_H

#include "core.h"

struct SimpleCPU {
    std::unique_ptr<SimpleCore> core = nullptr;

};


#endif