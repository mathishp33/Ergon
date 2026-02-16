#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "../utils.h"
#include "../error.h"
#include "decoder.h"

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
//Add instruction validation tables
//Add .data / .text sections
//Add register aliases (sp, pc, a0, etc.)
//️Add error underlining with exact token span


template <size_t PROGRAM_SIZE = 0xFFFF>
struct AsmInterpreter
{
    std::unordered_map<std::string, size_t> labels; // name and PC
    std::unordered_map<std::string, Var> vars; // name, addr and size
    std::vector<uint32_t> program;
    size_t max_lines = PROGRAM_SIZE;
    std::vector<std::string> lines;
    size_t cur_data_addr = 0;

    int get_var_addr(const std::string& var) {
        auto it = vars.find(var);
        if (it != vars.end())
            return static_cast<int>(it->second.addr);
        return -1;
    }

    ErrorCode decode_line(const std::string& line, uint32_t& result) {
        // labels
        if (line.ends_with(':')) {
            std::string label = line.substr(0, line.size() - 1);
            labels[label] = program.size() * 4; // PC
            return ErrorCode::OK;
        }

        const std::vector<std::string> tokens = string_utils::slice_str(line, ' ');

        auto it = instr_table.find(tokens[0]);
        if (it == instr_table.end()) {
            //var declaration
            if (tokens.size() < 2) return ErrorCode::UNKNOWN_INSTR;
            auto [error_code_4, var_size] = parse_DD(tokens[1]);
            if (error_code_4 != ErrorCode::OK) return error_code_4;
            Var v;
            v.addr = cur_data_addr;

            v.size = var_size;
            cur_data_addr += v.size;
            if (cur_data_addr >= program.size()) cur_data_addr = 0;

            vars[tokens[0]] = v;

            return ErrorCode::OK;
        }

        InstrDef def = it->second;

        // ---------- R-type ----------
        if (def.type == InstrType::R) {
            if (tokens.size() < 3) return ErrorCode::INVALID_ARG;

            auto [err_rd, rd] = parse_reg(tokens[1]);
            if (err_rd != ErrorCode::OK) return err_rd;

            auto [err_rs1, rs1] = parse_reg(tokens[2]);
            if (err_rs1 != ErrorCode::OK) return err_rs1;

            uint8_t rs2 = 0;
            if (tokens.size() > 3) {
                auto [err_rs2, rs2_val] = parse_reg(tokens[3]);
                if (err_rs2 != ErrorCode::OK) return err_rs2;
                rs2 = rs2_val;
            } else {
                if (!(it->first == "swap" || it->first == "mov"))
                    return ErrorCode::INVALID_ARG;
            }

            result = (static_cast<uint32_t>(def.opcode) << 24) | (rd << 16) | (rs1 << 8) | rs2;
            return ErrorCode::OK;
        }

        // ---------- I-type ----------
        if (def.type == InstrType::I) {
            if (tokens.size() < 3) return ErrorCode::INVALID_ARG;

            if (it->first == "ldb" || it->first == "ldh" || it->first == "ldw") {
                // ldX rd, addr
                if (tokens.size() != 3) return ErrorCode::INVALID_ARG;

                auto [e_rd, rd] = parse_reg(tokens[1]);
                if (e_rd != ErrorCode::OK) return e_rd;

                int addr = get_var_addr(tokens[2]);
                if (addr == -1) {
                    auto [e_imm, imm] = parse_imm(tokens[2]);
                    if (e_imm != ErrorCode::OK) return e_imm;
                    addr = imm;
                }

                result =
                    (uint32_t(def.opcode) << 24) | (rd << 16) | (0 << 8) | (addr & 0xFF); // base = 0

                return ErrorCode::OK;
            }
            if (it->first == "stb" || it->first == "sth" || it->first == "stw") {
                // stX rs, addr
                if (tokens.size() != 3) return ErrorCode::INVALID_ARG;

                auto [e_rs, rs] = parse_reg(tokens[1]);
                if (e_rs != ErrorCode::OK) return e_rs;

                int addr = get_var_addr(tokens[2]);
                if (addr == -1) {
                    auto [e_imm, imm] = parse_imm(tokens[2]);
                    if (e_imm != ErrorCode::OK) return e_imm;
                    addr = imm;
                }

                result = (uint32_t(def.opcode) << 24) | (rs << 16) | (0 << 8) | (addr & 0xFF); //base 0

                return ErrorCode::OK;
            }
            if (it->first == "lbaseb" || it->first == "lbaseh" || it->first == "lbasew") {
                // lbaseX rd, base, off
                if (tokens.size() != 4) return ErrorCode::INVALID_ARG;

                auto [e_rd, rd]   = parse_reg(tokens[1]);
                auto [e_base, rb] = parse_reg(tokens[2]);
                auto [e_off, off] = parse_imm(tokens[3]);

                if (e_rd != ErrorCode::OK || e_base != ErrorCode::OK || e_off != ErrorCode::OK) return ErrorCode::INVALID_ARG;

                result = (uint32_t(def.opcode) << 24) | (rd << 16) | (rb << 8) | (off & 0xFF);

                return ErrorCode::OK;
            }
            if (it->first == "sbaseb" || it->first == "sbaseh" || it->first == "sbasew") {
                // sbaseX rs, base, off
                if (tokens.size() != 4) return ErrorCode::INVALID_ARG;

                auto [e_rs, rs]   = parse_reg(tokens[1]);
                auto [e_base, rb] = parse_reg(tokens[2]);
                auto [e_off, off] = parse_imm(tokens[3]);

                if (e_rs != ErrorCode::OK || e_base != ErrorCode::OK || e_off != ErrorCode::OK) return ErrorCode::INVALID_ARG;

                result = (static_cast<uint32_t>(def.opcode) << 24) | (rs << 16) | (rb << 8) | (off & 0xFF);

                return ErrorCode::OK;
            }
            // (else)
            // Standard I-type: rd rs1 imm
            auto [err_rd, rd] = parse_reg(tokens[1]);
            if (err_rd != ErrorCode::OK) return err_rd;

            auto [err_rs1, rs1] = parse_reg(tokens[2]);
            if (err_rs1 != ErrorCode::OK) return err_rs1;

            // immediate could be a literal or a variable
            int imm = 0;
            int var_addr = get_var_addr(tokens[3]);
            if (var_addr != -1) {
                imm = var_addr;
            }
            else {
                auto [err_imm, imm_val] = parse_imm(tokens[3]);
                if (err_imm != ErrorCode::OK) return err_imm;
                imm = imm_val;
            }

            result = (static_cast<uint32_t>(def.opcode) << 24) | (rd << 16) | (rs1 << 8) | (imm & 0xFF);
            return ErrorCode::OK;
        }

        // ---------- J-type ----------
        if (def.type == InstrType::J) {
            if (it->first == "jmp" || it->first == "jz" || it->first == "jnz" || it->first == "jg"  || it->first == "jl" || it->first == "call") {
                if (tokens.size() != 2) return ErrorCode::INVALID_ARG;

                const std::string& label = tokens[1];
                if (!labels.contains(label)) return ErrorCode::UNKNOWN_LABEL;

                int offset = static_cast<int>(labels[label] - (program.size() * 4));
                result = (static_cast<uint32_t>(def.opcode) << 24) | (offset & 0xFFFFFF);
                return ErrorCode::OK;
                }

            if (it->first == "ret" || it->first == "halt") {
                result = (static_cast<uint32_t>(def.opcode) << 24);
                return ErrorCode::OK;
            }

            if (it->first == "inc" || it->first == "dec" || it->first == "clr" || it->first == "push" || it->first == "pop") {
                if (tokens.size() != 2) return ErrorCode::INVALID_ARG;

                auto [err, rd] = parse_reg(tokens[1]);
                if (err != ErrorCode::OK) return err;

                result = (static_cast<uint32_t>(def.opcode) << 24) | (rd << 16);
                return ErrorCode::OK;
            }

            // unary
            if (it->first == "not" || it->first == "abs" || it->first == "neg") {
                if (tokens.size() != 3) return ErrorCode::INVALID_ARG;

                auto [err_rd, rd] = parse_reg(tokens[1]);
                if (err_rd != ErrorCode::OK) return err_rd;

                auto [err_rs, rs] = parse_reg(tokens[2]);
                if (err_rs != ErrorCode::OK) return err_rs;

                result = (static_cast<uint32_t>(def.opcode) << 24) |
                         (rd << 16) | (rs << 8);
                return ErrorCode::OK;
            }

            return ErrorCode::INVALID_ARG;
        }

        return ErrorCode::UNKNOWN_INSTR;
    }

    ErrorInfo decode(const std::string& asm_program) {
        program.clear();

        lines = string_utils::slice_str(asm_program, '\n');
        if (lines.empty()) return {};
        if (lines.size() > max_lines) return { ErrorCode::LINE_OVERFLOW, lines.size() - 1};

        for (size_t i = 0; i < lines.size(); i++) {
            program.push_back(0);
            ErrorCode error_code = decode_line(string_utils::normalize(lines[i]), program.back());
            if (error_code != ErrorCode::OK)
                return {error_code, i};
        }

        return {};
    }
};


#endif