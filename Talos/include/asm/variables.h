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

inline size_t parse_DD(const std::string& str) {
    if (str == "DB" || str == "db") return static_cast<size_t>(DefineDirective::DB); //8b
    if (str == "DW" || str == "dw") return static_cast<size_t>(DefineDirective::DW); //16b
    if (str == "DD" || str == "dd") return static_cast<size_t>(DefineDirective::DD); //32b
    if (str == "DQ" || str == "dq") return static_cast<size_t>(DefineDirective::DQ);
    return 0;
}

inline size_t parse_RD(const std::string& str) {
    if (str == "RESB" || str == "resb") return static_cast<size_t>(ReserveDirective::RESB);
    if (str == "RESW" || str == "resw") return static_cast<size_t>(ReserveDirective::RESW);
    if (str == "RESD" || str == "resd") return static_cast<size_t>(ReserveDirective::RESD);
    if (str == "RESQ" || str == "resq") return static_cast<size_t>(ReserveDirective::RESQ);
    return 0;
}

inline std::pair<size_t, bool> parse_D(const std::string& str) {
    size_t dd = parse_DD(str);
    size_t rd = parse_RD(str);
    return { dd | rd, dd > rd ? 1 : 0 };
}

#endif