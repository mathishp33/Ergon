#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "utils.h"
#include "error.h"
#include "decoder.h"
#include "../COMPUTER/core.h"

#include <unordered_map>
#include <string>

/*
https://www.tutorialspoint.com/assembly_programming/assembly_registers.htm
www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
www.tutorialspoint.com/assembly_programming/assembly_variables.htm
VOIR AUTRES TUTOS sur www.tutorialspoint.com/assembly_programming


A FAIRE:

ADD Heap: malloc & free instructions
ADD .data, .text, .bss sections (and maybe other) (and maybe a linker mimic)
ADD register aliases (sp, pc, a0, etc.)
ADD 2 passes for labels
MEILLEURS ERREURS TYPES
FAIRE FPU
convert dispatcher to computed goto (use labels as values)
Unroll the dispatch loop
Superinstructions (merge addi+cmp+jl)
JIT (even minimal)
Profile-guided optimization (PGO) (maybe)
array access through: BYTE_TABLE[2] or BYTE_TABLE + 2
*/

template <size_t RAM_SIZE = 0x10000, size_t PROGRAM_SIZE = 0xFFFF>
struct AsmInterpreter {
    std::unordered_map<std::string, size_t> labels; // name and PC
    std::unordered_map<std::string, Var> vars; // name, addr and size
    std::vector<DecodedInstr> program;
    size_t max_lines = PROGRAM_SIZE;
    std::vector<std::string> lines;
    size_t cur_data_addr = 0;

    int get_var_addr(const std::string& var) {
        auto it = vars.find(var);
        if (it != vars.end())
            return static_cast<int>(it->second.addr);
        return -1;
    }

    ErrorCode decode_line(const std::string& line, DecodedInstr& result) {
        // labels
        if (line.ends_with(':')) {
            std::string label = line.substr(0, line.size() - 1);
            labels[label] = program.size(); // PC
            return ErrorCode::OK;
        }

        const std::vector<std::string> tokens = string_utils::slice_str(line, ' ');

        auto it = instr_table.find(tokens[0]);
        if (it == instr_table.end()) {
            //variable / array declaration
            if (tokens[1] == "times" || tokens[1] == "TIMES") {
                if (tokens.size() < 4) return ErrorCode::INVALID_ARG;
                //a changer (voir fonction dans decoder.h)
                auto [e_code_3, var_count] = parse_imm(tokens[2]);
                size_t var_size = parse_DD(tokens[3]);
                size_t un_var_size = parse_RD(tokens[3]);
                if (e_code_3 != ErrorCode::OK || (!var_size && !un_var_size)) return ErrorCode::INVALID_ARG;
                if (var_count < 1) return ErrorCode::INVALID_ARG;

                Var v(cur_data_addr, var_size | un_var_size, var_count);

                if (var_size) {
                    if (tokens.size() == 5) {
                        auto [e_init, bytes] = parse_bytes(tokens[4], v.size);
                        if (e_init != ErrorCode::OK) return e_init;
                        v.init = bytes;
                    }
                    else
                        return ErrorCode::INVALID_ARG;
                }

                if (cur_data_addr + v.size > RAM_SIZE)
                    return ErrorCode::RAM_OVERFLOW;
                cur_data_addr += v.size;

                vars[tokens[0]] = v;

                return ErrorCode::OK;
            }
            if (line.find(',') != std::string::npos) {
                //A FAIRE
                //ex: my_arr db 4, 6, 4, 2
            }
            if (tokens.size() < 2) return ErrorCode::UNKNOWN_INSTR;

            size_t var_size = parse_DD(tokens[1]);
            size_t un_var_size = parse_RD(tokens[1]);
            if (!var_size && !un_var_size) return ErrorCode::INVALID_ARG;

            Var v(cur_data_addr, var_size | un_var_size);
            if (var_size) {
                if (tokens.size() != 3)
                    return ErrorCode::INVALID_ARG;
                auto [e_code_6, res] = parse_bytes(tokens[2], v.init.size());
                if (e_code_6 != ErrorCode::OK) return e_code_6;
                v.init = res;
            }

            if (cur_data_addr + v.size > RAM_SIZE)
                return ErrorCode::RAM_OVERFLOW;
            cur_data_addr += v.size;

            vars[tokens[0]] = v;

            return ErrorCode::OK;
        }

        InstrDef def = it->second;

        if (tokens.size() - 1 != def.args.size())
            return ErrorCode::INVALID_ARG;

        std::array<uint8_t, 3> r{}; //rd, rs1, rs2
        int32_t imm = 0;

        for (size_t i = 0; i < def.args.size(); i++) {
            switch (def.args[i]) {
            case ArgType::REG:
                {
                    auto [err_rd, temp] = parse_reg(tokens[i + 1]);
                    if (err_rd != ErrorCode::OK) return err_rd;

                    r[def.args_pos[i]] = temp;
                    break;
                }
            case ArgType::IMM:
                {
                    auto [err_imm, imm_val] = parse_imm(tokens[i + 1]);
                    if (err_imm != ErrorCode::OK) return err_imm;

                    if (it->first == "movi")
                        imm = sign_extend_24b(imm_val);
                    else
                        r[def.args_pos[i]] = imm_val;
                    break;
                }
            case ArgType::LABEL:
                {
                    const std::string& label = tokens[i + 1];
                    if (!labels.contains(label)) return ErrorCode::UNKNOWN_LABEL;

                    //int offset = static_cast<int>(labels[label]) - static_cast<int>(program.size());
                    int offset = static_cast<int>(labels[label]) - static_cast<int>(program.size() - 1);
                    if (offset < -(1 << 23) || offset > ((1 << 23) - 1)) return ErrorCode::INVALID_ARG;

                    imm = sign_extend_24b(offset);
                    break;
                }
            case ArgType::VAR:
                {
                    int addr = get_var_addr(tokens[2]);
                    if (addr == -1) return ErrorCode::INVALID_ARG;
                    if (addr > 0xFFFF) return ErrorCode::INVALID_ARG;

                    imm = sign_extend_24b(addr);
                    break;
                }
            case ArgType::NONE:
                break;
            }
        }

        result.opcode = def.opcode;
        result.rd = r[0];
        result.rs1 = r[1];
        result.rs2 = r[2];
        result.imm = imm;

        return ErrorCode::OK;
    }

    ErrorInfo decode(const std::string& asm_program) {
        program.clear();

        lines = string_utils::slice_str(asm_program, '\n');
        if (lines.empty()) return {};
        if (lines.size() > max_lines) return { ErrorCode::LINE_OVERFLOW, lines.size() - 1};

        for (size_t i = 0; i < lines.size(); i++) {
            std::string cleaned_line = string_utils::normalize(lines[i]);
            if (cleaned_line.empty()) continue;

            program.push_back(DecodedInstr());
            ErrorCode error_code = decode_line(cleaned_line, program.back());
            if (error_code != ErrorCode::OK)
                return {error_code, i};
        }

        return {};
    }
};


#endif