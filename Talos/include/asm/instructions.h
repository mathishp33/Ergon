#ifndef ERGON_INSTRUCTIONS_H
#define ERGON_INSTRUCTIONS_H

#include <unordered_map>


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


#endif