#ifndef ERGON_DECODER_H
#define ERGON_DECODER_H

#include <unordered_map>

#include "../COMPUTER/core.h"


struct Var {
    size_t addr = 0;
    size_t size = 1;
    std::vector<uint8_t> init;
    size_t elem_count = 1;

    Var() = default;

    Var(size_t addr, size_t size) : addr(addr), size(size) {
        for (size_t i = 0; i < size; i++) init.push_back(0);
        elem_count = 1;
    }

    Var(size_t addr, size_t size, size_t elem_count) : addr(addr), size(size), elem_count(elem_count) {
        for (size_t i = 0; i < size * elem_count; i++) init.push_back(0);
    }
};

enum class InstrType { R, I, J };

enum class ArgType {
    REG,
    IMM,
    LABEL,
    VAR,
    NONE
};

struct InstrDef {
    OPCODE opcode;
    InstrType type;
    std::vector<ArgType> args;
    std::vector<size_t> args_pos;

    InstrDef(OPCODE opcode, InstrType type, const std::vector<ArgType>& args, const std::vector<size_t>& args_pos) : opcode(opcode), type(type), args(args), args_pos(args_pos) {
        if (args.size() != args_pos.size()) throw std::exception();
    }
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
    {"add",  {ADD,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }}, //rd, rs1, rs2/imm
    {"sub",  {SUB,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"mul",  {MUL,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"div",  {DIV,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"mod",  {MOD,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"addi", {ADDI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"subi", {SUBI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"muli", {MULI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"divi", {DIVI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"modi", {MODI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},

    {"and",  {AND,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"or",   {OR,   InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"xor",  {XOR,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"andi", {ANDI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"ori",  {ORI,  InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"xori", {XORI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},

    {"shl",  {SHL,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"shr",  {SHR,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"sar",  {SAR,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"rol",  {ROL,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"ror",  {ROR,  InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"shli", {SHLI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"shri", {SHRI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"sari", {SARI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"roli", {ROLI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"rori", {RORI, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},

    {"cmp",   {CMP,   InstrType::R, { ArgType::REG, ArgType::REG }, { 1, 2 } }}, //rs1, rs2/imm
    {"cmpu",  {CMPU,  InstrType::R, { ArgType::REG, ArgType::REG }, { 1, 2 } }},
    {"test",  {TEST,  InstrType::R, { ArgType::REG, ArgType::REG }, { 1, 2 } }},
    {"cmpi",  {CMPI,  InstrType::I, { ArgType::REG, ArgType::IMM }, { 1, 2 } }},
    {"cmpui", {CMPUI, InstrType::I, { ArgType::REG, ArgType::IMM }, { 1, 2 } }},
    {"testi", {TESTI, InstrType::I, { ArgType::REG, ArgType::IMM }, { 1, 2 } }},

    {"inc", {INC, InstrType::J, { ArgType::REG }, { 0 } }}, //rd
    {"dec", {DEC, InstrType::J, { ArgType::REG }, { 0 } }},
    {"not", {NOT, InstrType::J, { ArgType::REG, ArgType::REG }, { 0, 1 } }},
    {"abs", {ABS, InstrType::J, { ArgType::REG, ArgType::REG }, { 0, 1 } }},
    {"neg", {NEG, InstrType::J, { ArgType::REG, ArgType::REG }, { 0, 1 } }},
    {"min", {MIN, InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"max", {MAX, InstrType::R, { ArgType::REG, ArgType::REG, ArgType::REG }, { 0, 1, 2 } }},
    {"mini",{MINI,InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"maxi",{MAXI,InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},

    {"mov",   {MOV_REG, InstrType::R, { ArgType::REG, ArgType::REG }, { 0, 1 } }},
    {"movi",  {MOV_IMM, InstrType::I, { ArgType::REG, ArgType::IMM }, { 0, 2 } }},
    {"lbaseb", {LDB_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"lbaseh", {LDH_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"lbasew", {LDW_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"sbaseb", {STB_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"sbaseh", {STH_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"sbasew", {STW_BASE, InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"ldb", {LDB_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"ldh", {LDH_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"ldw", {LDW_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"stb", {STB_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"sth", {STH_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"stw", {STW_ABS, InstrType::I, { ArgType::REG, ArgType::VAR }, { 0, 2 } }},
    {"push",  {PUSH,    InstrType::J, { ArgType::REG }, { 1 } }}, //rs1
    {"pop",   {POP,     InstrType::J, { ArgType::REG }, { 0 } }}, //rd
    {"lea",   {LEA,     InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},
    {"swap",  {SWAP,    InstrType::R, { ArgType::REG, ArgType::REG }, { 0, 1 } }},
    {"clr",   {CLR,     InstrType::J, { ArgType::REG }, { 0 } }},
    {"memcpy",{MEMCPY,  InstrType::I, { ArgType::REG, ArgType::REG, ArgType::IMM }, { 0, 1, 2 } }},

    {"jmp",  {JMP,  InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"jz",   {JZ,   InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"jnz",  {JNZ,  InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"jg",   {JG,   InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"jl",   {JL,   InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"call", {CALL, InstrType::J, { ArgType::LABEL }, { 0 } }},
    {"ret",  {RET,  InstrType::J, { }, { } }},

    {"halt",  {HALT,  InstrType::J, { }, { } }},
};


inline std::unordered_map<std::string, uint8_t> reg_table = {
    { "zero", 0 },
    { "eax", 1 },
    { "ebx", 2 },
    { "ecx", 3 },
    { "edx", 4 },
    { "esi", 5 },
    { "edi", 6 },
    { "r7", 7 },
    { "r8", 8 },
    { "r9", 9 },
    { "r10", 10 },
    { "r11", 11 },
    { "r12", 12 },
    { "sp", 13 },
    { "bp", 14 },
    { "r15", 15 }
};

static std::pair<ErrorCode, uint8_t> parse_reg(const std::string& s) {
    if (reg_table.contains(s))
        return { ErrorCode::OK, reg_table[s] };
    return { ErrorCode::INVALID_ARG, 0 };
}

static std::pair<ErrorCode, int> parse_imm(const std::string& s) {
    if (s.starts_with("0x"))
        return string_utils::better_stoi(s.substr(2), nullptr, 16);
    if (s.starts_with("0b"))
        return string_utils::better_stoi(s.substr(2), nullptr, 2);
    return string_utils::better_stoi(s);
}

inline std::pair<ErrorCode, std::vector<uint8_t>> parse_bytes(const std::string& s, size_t final_size) {
    if (final_size == 0)
        return { ErrorCode::OK, {} };

    std::vector<uint8_t> bytes;
    bytes.reserve(final_size);

    //BASE 16
    if (s.starts_with("0x")) {
        std::string hex = s.substr(2);

        // 2 hex chars = 1 byte
        if (hex.size() != final_size * 2)
            return { ErrorCode::INVALID_ARG, {} };

        for (size_t i = 0; i < hex.size(); i += 2) {
            auto [ec, value] =
                string_utils::better_stoi(hex.substr(i, 2), nullptr, 16);

            if (ec != ErrorCode::OK || value > 0xFF)
                return { ErrorCode::INVALID_ARG, {} };

            bytes.push_back(static_cast<uint8_t>(value));
        }
        return { ErrorCode::OK, bytes };
    }

    //BASE 2
    if (s.starts_with("0b")) {
        std::string bin = s.substr(2);

        // 8 bits = 1 byte
        if (bin.size() != final_size * 8)
            return { ErrorCode::INVALID_ARG, {} };

        for (size_t i = 0; i < bin.size(); i += 8) {
            auto [ec, value] =
                string_utils::better_stoi(bin.substr(i, 8), nullptr, 2);

            if (ec != ErrorCode::OK || value > 0xFF)
                return { ErrorCode::INVALID_ARG, {} };

            bytes.push_back(static_cast<uint8_t>(value));
        }
        return { ErrorCode::OK, bytes };
    }

    //STRING / CHAR LITERAL
    if ((s.starts_with("\"") && s.ends_with("\"")) || (s.starts_with("'")  && s.ends_with("'"))) {
        std::string content = s.substr(1, s.size() - 2);

        if (content.size() != final_size)
            return { ErrorCode::INVALID_ARG, {} };

        for (unsigned char c : content)
            bytes.push_back(static_cast<uint8_t>(c));

        return { ErrorCode::OK, bytes };
    }


    //BASE 10
    {
        auto [ec, value] = string_utils::better_stoi(s, nullptr, 10);
        if (ec != ErrorCode::OK)
            return { ec, {} };

        for (size_t i = 0; i < final_size; ++i) {
            bytes.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
        }
        return { ErrorCode::OK, bytes };
    }
}

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

inline std::pair<size_t, int> parse_D(const std::string& str) {
    size_t dd = parse_DD(str);
    size_t rd = parse_RD(str);
    return { dd | rd, dd > rd ? dd : rd };
}

static int32_t sign_extend_24b(uint32_t value) {
    if (value & 0x800000) // bit 23 = sign
        return static_cast<int32_t>(value | 0xFF000000); // extend sign
    return static_cast<int32_t>(value);
}

#endif