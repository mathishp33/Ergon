#ifndef ERGON_DECODER_H
#define ERGON_DECODER_H

#include <unordered_map>


struct Var {
    size_t addr = 0;
    size_t size = 0;
};

enum class InstrType { R, I, J };

struct InstrDef {
    OPCODE opcode;
    InstrType type;
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

inline std::unordered_map<std::string, InstrDef> instr_table = {
    {"add",  {ADD,  InstrType::R}},
    {"sub",  {SUB,  InstrType::R}},
    {"mul",  {MUL,  InstrType::R}},
    {"div",  {DIV,  InstrType::R}},
    {"mod",  {MOD,  InstrType::R}},
    {"addi", {ADDI, InstrType::I}},
    {"subi", {SUBI, InstrType::I}},
    {"muli", {MULI, InstrType::I}},
    {"divi", {DIVI, InstrType::I}},
    {"modi", {MODI, InstrType::I}},

    {"and",  {AND,  InstrType::R}},
    {"or",   {OR,   InstrType::R}},
    {"xor",  {XOR,  InstrType::R}},
    {"andi", {ANDI, InstrType::I}},
    {"ori",  {ORI,  InstrType::I}},
    {"xori", {XORI, InstrType::I}},

    {"shl",  {SHL,  InstrType::R}},
    {"shr",  {SHR,  InstrType::R}},
    {"sar",  {SAR,  InstrType::R}},
    {"rol",  {ROL,  InstrType::R}},
    {"ror",  {ROR,  InstrType::R}},
    {"shli", {SHLI, InstrType::I}},
    {"shri", {SHRI, InstrType::I}},
    {"sari", {SARI, InstrType::I}},
    {"roli", {ROLI, InstrType::I}},
    {"rori", {RORI, InstrType::I}},

    {"cmp",   {CMP,   InstrType::R}},
    {"test",  {TEST,  InstrType::R}},
    {"cmpi",  {CMPI,  InstrType::I}},
    {"testi", {TESTI, InstrType::I}},

    {"inc", {INC, InstrType::J}},
    {"dec", {DEC, InstrType::J}},
    {"not", {NOT, InstrType::J}},
    {"abs", {ABS, InstrType::J}},
    {"neg", {NEG, InstrType::J}},
    {"min", {MIN, InstrType::R}},
    {"max", {MAX, InstrType::R}},
    {"minI",{MINI,InstrType::I}},
    {"maxI",{MAXI,InstrType::I}},

    {"mov",   {MOV_REG, InstrType::R}},
    {"movi",  {MOV_IMM, InstrType::I}},
    {"load",  {LOAD,    InstrType::I}},
    {"store", {STORE,   InstrType::I}},
    { "lbase",{LOAD_BASE, InstrType::I}},
    { "sbase",{STORE_BASE, InstrType::I}},
    {"loadb", {LDB,     InstrType::I}},
    {"loadh", {LDH,     InstrType::I}},
    {"loadw", {LDW,     InstrType::I}},
    {"stb",   {STB,     InstrType::I}},
    {"sth",   {STH,     InstrType::I}},
    {"stw",   {STW,     InstrType::I}},
    {"push",  {PUSH,    InstrType::J}},
    {"pop",   {POP,     InstrType::J}},
    {"lea",   {LEA,     InstrType::I}},
    {"swap",  {SWAP,    InstrType::R}},
    {"clr",   {CLR,     InstrType::J}},
    {"memcpy",{MEMCPY,  InstrType::I}},

    {"jmp",  {JMP,  InstrType::J}},
    {"jz",   {JZ,   InstrType::J}},
    {"jnz",  {JNZ,  InstrType::J}},
    {"jg",   {JG,   InstrType::J}},
    {"jl",   {JL,   InstrType::J}},
    {"call", {CALL, InstrType::J}},
    {"ret",  {RET,  InstrType::J}},

    {"halt",  {HALT,  InstrType::J}},
};


static std::pair<ErrorCode, uint8_t> parse_reg(const std::string& s) {
    size_t offset = 0;
    if (s[0] == 'R' || s[0] == 'r' || s[0] == '%') offset = 1;
    return string_utils::better_stoi(s.substr(offset));
}

static std::pair<ErrorCode, int> parse_imm(const std::string& s) {
    if (s.starts_with("0x"))
        return string_utils::better_stoi(s.substr(2), nullptr, 16);
    if (s.starts_with("0b"))
        return string_utils::better_stoi(s.substr(2), nullptr, 2);
    return string_utils::better_stoi(s);
}

inline std::pair<ErrorCode, size_t> parse_DD(const std::string& str) {
    if (str == "DB") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DB)};
    if (str == "DW") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DW)};
    if (str == "DD") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DD)};
    if (str == "DQ") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DQ)};
    return { ErrorCode::INVALID_ARG, 0 };
}


#endif