#ifndef ERGON_VARIABLES_H
#define ERGON_VARIABLES_H

#include <string>


struct Var {
    size_t addr = 0;
    size_t size = 1;
    size_t elem_count = 1;

    Var() = default;

    Var(size_t addr, size_t size) : addr(addr), size(size) {}

    Var(size_t addr, size_t size, size_t elem_count) : addr(addr), size(size), elem_count(elem_count) {}
};

// sizes in bytes
enum class DefineDirective : unsigned int {
    DB = 1,  // Define Byte
    DW = 2,  // Define Word (2 bytes)
    DD = 4,  // Define Doubleword (4 bytes)
    DQ = 8,  // Define Quadword (8 bytes)
    //DT = 10, // Define Ten Bytes (custom) REQUIRE 64-bit
};

// sizes in bytes
enum class ReserveDirective : unsigned int {
    RESB = 1,  // Reserve Byte
    RESW = 2,  // Reserve Word (2 bytes)
    RESD = 4,  // Reserve Doubleword (4 bytes)
    RESQ = 8,  // Reserve Quadword (8 bytes)
    //REST = 10, // Reserve Ten Bytes (custom) REQUIRE 64-bit
};

#endif