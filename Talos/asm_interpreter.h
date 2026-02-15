#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "core.h"
#include "utils.h"

#include <unordered_map>
#include <string>
#include <system_error>

struct Var {
    size_t addr;
    size_t size;
};

enum class InstrType { R, I, J };

struct InstrDef {
    OPCODE opcode;
    InstrType type;
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


/*
ERROR CODES:
0 = no error
1 = unknown instruction
2 = invalid argument for instruction
3 = unknown label
4 = invalid character (stoi error)
5 = overflow (stoi error)
 */
// there is padding, but I don't want to #pragma pack(1) bc it gives warning
struct ErrorInfo {
    int error_code = 0;
    size_t index_line = 0;

    ErrorInfo();
    ErrorInfo(int e_code, size_t i) {
        error_code = e_code;
        index_line = i;
    }
};

struct AsmInterpreter {
    std::unordered_map<std::string, size_t> labels; // name and PC
    std::unordered_map<std::string, Var> vars; // name, addr and size
    std::vector<uint32_t> program;
    size_t cur_data_addr = 0;

    static std::pair<int, uint8_t> parse_reg(const std::string& s) {
        if (s[0] != 'R' && s[0] != 'r') return { 4, 0 };
        return better_stoi(s.substr(1));
    }

    static std::pair<int, int> parse_imm(const std::string& s) {
        if (s.starts_with("0x"))
            return better_stoi(s.substr(2), nullptr, 16);
        if (s.starts_with("0b"))
            return better_stoi(s.substr(2), nullptr, 2);
        return better_stoi(s);
    }

    int decode_line(const std::string& line, uint32_t& result) {
        if (line.ends_with(':')) {
            std::string label = line.substr(0, line.size() - 1);
            labels[label] = program.size() * 4; // PC
            return 0;
        }
        const std::vector<std::string> tokens = slice_str(line, ' ');
        if (tokens[0] == "var") {
            if (tokens.size() != 3) return 3;

            Var v;
            v.addr = cur_data_addr;
            v.size = std::stoi(tokens[2]);

            vars[tokens[1]] = v;
            cur_data_addr += v.size;
            return 0;
        }
        auto it = instr_table.find(tokens[0]);
        if (it == instr_table.end())
            return 1; // unknown instruction

        InstrDef def = it->second;

        if (def.type == InstrType::R) {
            //exceptions are MOV_REG and SWAP
            if (tokens.size() > 2) {
                auto [error_code_1, rd] = parse_reg(tokens[1]);
                if (!error_code_1) return error_code_1;
                auto [error_code_2, rs1] = parse_reg(tokens[2]);
                if (!error_code_2) return error_code_2;
            } else
                return 2;

            uint8_t rs2 = 0;
            if ((it->first != "swap" || it->first != "mov") && tokens.size() > 3) {
                auto [error_code_3, loc_rs2] = parse_reg(tokens[3]);
                if (!error_code_3) return error_code_3;
                rs2 = loc_rs2;
            }
            else
                return 2;

            result = (static_cast<uint32_t>(def.opcode) << 24) | (rd << 16) | (rs1 << 8) | rs2;
        }
        if (def.type == InstrType::I) {
            if (tokens.size() > 3) {
                auto [error_code_1, rd] = parse_reg(tokens[1]);
                if (!error_code_1) return error_code_1;
                auto [error_code_2, rs1] = parse_reg(tokens[2]);
                if (!error_code_2) return error_code_2;
                auto [error_code_3, imm] = parse_imm(tokens[3]);
                if (!error_code_3) return error_code_3;
            } else
                return 2;

            result = (static_cast<uint32_t>(def.opcode) << 24) | (rd  << 16) | (rs1 << 8) | (imm & 0xFF);
        }
        if (def.type == InstrType::J) {
            /* IMPLEMENT LABEL TO PC CONVERTER
            "jmp",
            "jz",
            "jnz",
            "jg",
            "jl",
            "call",
            "ret",
            "halt"
            */
            std::string label = tokens[1];

            if (!labels.contains(label))
                return 4; // unknown label

            int offset = labels[label] - (program.size() * 4);

            result =
                (static_cast<uint32_t>(def.opcode) << 24) |
                (offset & 0xFFFFFF);
        }

        return 0;
    }

    std::string normalize(std::string line) {
        //comments
        if (auto pos = line.find(';'); pos != std::string::npos)
            line = line.substr(0, pos);

        //trim spaces
        auto l = line.find_first_not_of(" \t");
        auto r = line.find_last_not_of(" \t");
        if (l == std::string::npos) return "";
        return line.substr(l, r - l + 1);
    }

    ErrorInfo decode(const std::string& asm_program) {
        program.clear();
        const std::vector<std::string> lines = slice_str(asm_program, '\n');
        for (size_t i = 0; i < lines.size(); i++) {
            program.push_back(0);
            int error_code = decode_line(lines[i], program.back());
            if (!error_code)
                return ErrorInfo(error_code, i);
        }

        return ErrorInfo();
    }
};


#endif