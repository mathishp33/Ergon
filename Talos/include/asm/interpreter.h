#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "error.h"
#include "decoder.h"
#include "instructions.h"
#include "variables.h"
#include "parser.h"
#include "../computer/core.h"

#include <unordered_map>
#include <string>

/*
www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
VOIR AUTRES TUTOS sur www.tutorialspoint.com/assembly_programming


A FAIRE:

ADD Heap: malloc & free instructions (.heap & .stack sections)
ADD .global & .extern sections, linker steps and maybe ELF layout
FAIRE FPU
convert dispatcher to computed goto (use labels as values)
Unroll the dispatch loop
Super-instructions (merge addi+cmp+jl)
AJOUTER JIT
Profile-guided optimization (PGO) (maybe)
AJOUTER acès array avec: BYTE_TABLE[2] or BYTE_TABLE + 2
AJOUTER constantes
AJOUTER solveur numérique pour les imm (ex: 0x3 + 0b11001 * (-133))
*/

template <size_t RAM_SIZE = 0x10000, size_t PROGRAM_SIZE = 0xFFFF>
struct AsmInterpreter {
    std::unordered_map<std::string, Label> labels;
    std::unordered_map<std::string, Var> vars;
    std::unordered_map<std::string, Symbol> symbols;
    size_t max_lines = PROGRAM_SIZE;
    std::vector<std::string> lines;
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    size_t bss_size = 0;
    Section cur_section = Section::TEXT;
    size_t cur_pc = 0;

    int get_var_addr(const std::string& var) {
        auto it = vars.find(var);
        if (it != vars.end())
            return static_cast<int>(it->second.addr);
        return -1;
    }

        ErrorCode decode_line(const std::string& line) {
        //sections
        if (line == ".text") {
            cur_section = Section::TEXT;
            return ErrorCode::OK;
        }
        if (line == ".data") {
            cur_section = Section::DATA;
            return ErrorCode::OK;
        }
        if (line == ".bss") {
            cur_section = Section::BSS;
            return ErrorCode::OK;
        }

        //labels
        if (line.ends_with(':')) return ErrorCode::OK;

        const std::vector<std::string> tokens = string_utils::slice_str(line, ' ');

        auto it = instr_table.find(tokens[0]);

        if (cur_section != Section::TEXT) return ErrorCode::OK;

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

                    const Label& L = labels[label];
                    if (L.section != Section::TEXT)
                        return ErrorCode::INVALID_LABEL_SECTION;

                    int32_t offset = static_cast<int32_t>(L.value) - (cur_pc + 1);

                    if (offset < -(1 << 23) || offset > ((1 << 23) - 1))
                        return ErrorCode::INVALID_ARG;

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

        DecodedInstr result = DecodedInstr(def.opcode, r[0], r[1], r[2], imm);

        cur_pc++;
        text.emplace_back(result);

        return ErrorCode::OK;
    }

    ErrorInfo first_pass() {
        cur_section = Section::TEXT;
        size_t text_pc = 0;

        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = string_utils::normalize(lines[i]);
            if (line.empty()) continue;

            // sections
            if (line == ".text") { cur_section = Section::TEXT; continue; }
            if (line == ".data") { cur_section = Section::DATA; continue; }
            if (line == ".bss") { cur_section = Section::BSS; continue; }

            // labels
            if (line.ends_with(':')) {
                std::string name = line.substr(0, line.size() - 1);

                if (labels.contains(name)) return { ErrorCode::DUPLICATE_LABEL, i };

                size_t value = (cur_section == Section::TEXT) ? text_pc : (cur_section == Section::DATA) ? data.size() : bss_size;

                labels[name] = { cur_section, value };
                continue;
            }

            //variable / array declaration
            auto tokens = string_utils::slice_str(line, ' ');
            auto it = instr_table.find(tokens[0]);

            if (it == instr_table.end()) {
                if (tokens[1] == "times" || tokens[1] == "TIMES") {
                    if (tokens.size() < 4) return { ErrorCode::INVALID_ARG, i };
                    auto [e_code_3, var_count] = parse_imm(tokens[2]);
                    auto [var_size, needs_init] = parse_D(tokens[3]);
                    if (e_code_3 != ErrorCode::OK || !var_size) return { ErrorCode::INVALID_ARG, i };
                    if (var_count < 1) return { ErrorCode::INVALID_ARG, i };

                    Var v(needs_init ? data.size() : bss_size, var_size, var_count);

                    if (needs_init) {
                        if (cur_section != Section::DATA) return { ErrorCode::VAR_OUTSIDE_DATA, i };
                        if (tokens.size() != 5) return { ErrorCode::INVALID_ARG, i };

                        auto [e_init, bytes] = parse_bytes(tokens[4], v.size);
                        if (e_init != ErrorCode::OK) return { e_init, i };

                        data.insert(data.end(), bytes.begin(), bytes.end());
                    }
                    else {
                        if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, i };
                        bss_size += var_size;
                    }

                    vars[tokens[0]] = v;
                    continue;
                }
                if (line.find(',') != std::string::npos) {
                    //A FAIRE
                    //ex: my_arr db 4, 6, 4, 2
                }
                if (tokens.size() < 2) return { ErrorCode::INVALID_ARG, i };

                auto [var_size, needs_init] = parse_D(tokens[1]);
                if (!var_size) return { ErrorCode::INVALID_ARG, i };

                Var v(needs_init ? data.size() : bss_size, var_size);
                if (needs_init) {
                    if (cur_section != Section::DATA) return { ErrorCode::VAR_OUTSIDE_DATA, i };
                    if (tokens.size() != 3) return { ErrorCode::INVALID_ARG, i };

                    auto [e_code_6, res] = parse_bytes(tokens[2], var_size);
                    if (e_code_6 != ErrorCode::OK) return { e_code_6, i };

                    data.insert(data.end(), res.begin(), res.end());
                }
                else {
                    if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, i };
                    bss_size += var_size;
                }

                vars[tokens[0]] = v;
                continue;
            }

            // instructions only advance PC in .text
            if (cur_section == Section::TEXT)
                text_pc++;
        }

        return {};
    }

    ErrorInfo decode(const std::string& asm_program) {
        text.clear();
        labels.clear();
        vars.clear();
        data.clear();
        bss_size = 0;
        cur_pc = 0;

        lines = string_utils::slice_str(asm_program, '\n');
        if (lines.size() > max_lines) return { ErrorCode::LINE_OVERFLOW, lines.size() - 1 };

        auto e_info_f_pass = first_pass();
        if (e_info_f_pass.error_code != ErrorCode::OK)
            return e_info_f_pass;

        cur_section = Section::TEXT;

        if (lines.empty()) return {};
        if (lines.size() > max_lines) return { ErrorCode::LINE_OVERFLOW, lines.size() - 1};


        for (size_t i = 0; i < lines.size(); i++) {
            std::string cleaned_line = string_utils::normalize(lines[i]);
            if (cleaned_line.empty()) continue;

            ErrorCode e_code_text = decode_line(cleaned_line);
            if (e_code_text != ErrorCode::OK)
                return {e_code_text, i};
        }

        if (data.size() > RAM_SIZE) return { ErrorCode::RAM_OVERFLOW, 0 };

        return {};
    }

    ObjectFile get_object_file() {

    }

    std::pair<ObjectFile, ErrorInfo> assemble() {

    }
};


#endif