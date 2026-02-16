#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "core.h"
#include "utils.h"
#include "error.h"

#include <unordered_map>
#include <string>

//A FAIRE:
//Heap: malloc & free instructions
//give registers names
//maybe adds linker and section
//https://www.tutorialspoint.com/assembly_programming/assembly_registers.htm
//www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
//www.tutorialspoint.com/assembly_programming/assembly_variables.htm
//voir autres tuto sur www.tutorialspoint.com/assembly_programming

struct Var {
    size_t addr = 0;
    size_t size = 0;
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


template <size_t PROGRAM_SIZE = 0xFFFF>
struct AsmInterpreter {
    std::unordered_map<std::string, size_t> labels; // name and PC
    std::unordered_map<std::string, Var> vars; // name, addr and size
    std::vector<uint32_t> program;
    size_t max_lines = PROGRAM_SIZE;
    std::vector<std::string> lines;
    size_t cur_data_addr = 0;

    static std::pair<ErrorCode, uint8_t> parse_reg(const std::string& s) {
        size_t offset = 0;
        if (s[0] == 'R' || s[0] == 'r' || s[0] == '%') offset = 1;
        return better_stoi(s.substr(offset));
    }

    int get_var_addr(const std::string& var) {
        auto it = vars.find(var);
        if (it != vars.end())
            return static_cast<int>(it->second.addr);
        return -1;
    }

    static std::pair<ErrorCode, int> parse_imm(const std::string& s) {
        if (s.starts_with("0x"))
            return better_stoi(s.substr(2), nullptr, 16);
        if (s.starts_with("0b"))
            return better_stoi(s.substr(2), nullptr, 2);
        return better_stoi(s);
    }

    ErrorCode decode_line(const std::string& line, uint32_t& result) {
        if (line.ends_with(':')) {
            std::string label = line.substr(0, line.size() - 1);
            labels[label] = program.size() * 4; // PC
            return ErrorCode::OK;
        }
        const std::vector<std::string> tokens = slice_str(line, ' ');
        if (tokens[0] == "var") {
            if (tokens.size() != 3) return ErrorCode::UNKNOWN_LABEL;

            Var v;
            v.addr = cur_data_addr;
            v.size = std::stoi(tokens[2]);

            vars[tokens[1]] = v;
            cur_data_addr += v.size;
            return ErrorCode::OK;
        }
        auto it = instr_table.find(tokens[0]);
        if (it == instr_table.end())
            return ErrorCode::UNKNOWN_INSTR;

        InstrDef def = it->second;

        if (def.type == InstrType::R) {
            //exceptions are MOV_REG and SWAP
            if (tokens.size() > 2) {
                auto [error_code_1, rd] = parse_reg(tokens[1]);
                if (error_code_1 != ErrorCode::OK) return error_code_1;
                auto [error_code_2, rs1] = parse_reg(tokens[2]);
                if (error_code_2 != ErrorCode::OK) return error_code_2;

                uint8_t rs2 = 0;
                if (tokens.size() > 3) {
                    auto [error_code_3, loc_rs2] = parse_reg(tokens[3]);
                    if (error_code_3 != ErrorCode::OK) return error_code_3;
                    rs2 = loc_rs2;
                }
                else {
                    if (!(it->first == "swap" || it->first == "mov"))
                        return ErrorCode::INVALID_ARG;
                }
                result = (static_cast<uint32_t>(def.opcode) << 24) | (rd << 16) | (rs1 << 8) | rs2;
            } else
                return ErrorCode::INVALID_ARG;
        }
        if (def.type == InstrType::I) {
            if (tokens.size() > 3) {
                auto [error_code_1, rd] = parse_reg(tokens[1]);
                if (error_code_1 != ErrorCode::OK) return error_code_1;

                auto [error_code_2, rs1] = parse_reg(tokens[2]);
                if (error_code_2 != ErrorCode::OK) return error_code_2;

                int imm = 0;
                auto var_addr = get_var_addr(tokens[3]);
                if (var_addr != -1) {
                    imm = var_addr;
                } else {
                    auto [error_code_3, imm_val] = parse_imm(tokens[3]);
                    if (error_code_3 != ErrorCode::OK) return error_code_3;
                    imm = imm_val;
                }

                result = (static_cast<uint32_t>(def.opcode) << 24) | (rd  << 16) | (rs1 << 8) | (imm & 0xFF);
            } else
                return ErrorCode::INVALID_ARG;
        }
        if (def.type == InstrType::J) {
            const std::string& label = tokens[1];

            if (!labels.contains(label))
                return ErrorCode::UNKNOWN_LABEL;

            int offset = static_cast<int>(labels[label] - (program.size() * 4));

            result = (static_cast<uint32_t>(def.opcode) << 24) | (offset & 0xFFFFFF);
        }

        return ErrorCode::OK;
    }

    static std::string normalize(std::string line) {
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

        lines = slice_str(asm_program, '\n');
        if (lines.empty()) return {};
        if (lines.size() > max_lines) return { ErrorCode::LINE_OVERFLOW, lines.size() - 1};

        for (size_t i = 0; i < lines.size(); i++) {
            program.push_back(0);
            ErrorCode error_code = decode_line(normalize(lines[i]), program.back());
            if (error_code != ErrorCode::OK)
                return {error_code, i};
        }

        return {};
    }
};


#endif