#ifndef ERGON_DECODER_H
#define ERGON_DECODER_H

#include <unordered_map>


struct Var {
    size_t addr = 0;
    size_t size = 0;
    std::vector<uint8_t> init;

    Var() = default;

    Var(size_t addr, size_t size) : addr(addr), size(size) {
        for (size_t i = 0; i < size; i++) init.push_back(0);
    }
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
    {"mini",{MINI,InstrType::I}},
    {"maxi",{MAXI,InstrType::I}},

    {"mov",   {MOV_REG, InstrType::R}},
    {"movi",  {MOV_IMM, InstrType::I}},
    {"lbaseb", {LDB_BASE, InstrType::I}},
    {"lbaseh", {LDH_BASE, InstrType::I}},
    {"lbasew", {LDW_BASE, InstrType::I}},
    {"sbaseb", {STB_BASE, InstrType::I}},
    {"sbaseh", {STH_BASE, InstrType::I}},
    {"sbasew", {STW_BASE, InstrType::I}},
    {"ldb", {LDB_ABS, InstrType::I}},
    {"ldh", {LDH_ABS, InstrType::I}},
    {"ldw", {LDW_ABS, InstrType::I}},
    {"stb", {STB_ABS, InstrType::I}},
    {"sth", {STH_ABS, InstrType::I}},
    {"stw", {STW_ABS, InstrType::I}},
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

        // Fit value into final_size bytes (big-endian)
        for (size_t i = 0; i < final_size; ++i) {
            size_t shift = (final_size - 1 - i) * 8;
            bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFF));
        }
        return { ErrorCode::OK, bytes };
    }
}

inline std::pair<ErrorCode, size_t> parse_DD(const std::string& str) {
    if (str == "DB") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DB)};
    if (str == "DW") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DW)};
    if (str == "DD") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DD)};
    if (str == "DQ") return { ErrorCode::OK, static_cast<size_t>(DefineDirective::DQ)};
    return { ErrorCode::INVALID_ARG, 0 };
}

inline std::pair<ErrorCode, size_t> parse_RD(const std::string& str) {
    if (str == "RESB") return { ErrorCode::OK, static_cast<size_t>(ReserveDirective::RESB) };
    if (str == "RESW") return { ErrorCode::OK, static_cast<size_t>(ReserveDirective::RESW) };
    if (str == "RESD") return { ErrorCode::OK, static_cast<size_t>(ReserveDirective::RESD) };
    if (str == "RESQ") return { ErrorCode::OK, static_cast<size_t>(ReserveDirective::RESQ) };
    return { ErrorCode::INVALID_ARG, 0 };
}


#endif