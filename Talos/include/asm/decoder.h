#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "error.h"
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


enum class Section {
    TEXT, // instructions
    DATA, // initialized data
    //HEAP, //
    //STACK, //
    RODATA, // constants
    BSS, // un-initialized data
    NONE
};

enum class SymbolBinding {
    LOCAL,
    GLOBAL,
    EXTERN
};

struct Label {
    Section section;
    size_t value;
};

struct Symbol {
    std::string name;
    Section section;
    uint32_t value; // offset in section
    SymbolBinding bind;
};

enum class RelocType {
    PC_REL_24, // jumps, calls
    ABS_32 // data addresses
};

struct Relocation {
    Section section;
    uint32_t offset; // where to patch
    RelocType type;
    std::string symbol;
};

struct ObjectFile {
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    std::vector<uint8_t> rodata;
    uint32_t bss_size = 0;

    uint32_t text_base = 0;
    uint32_t data_base = 0;
    uint32_t rodata_base = 0;
    uint32_t bss_base  = 0;

    std::unordered_map<std::string, Symbol> symbols;
    std::vector<Relocation> relocations;

    std::string entry_symbol;
};


static int32_t sign_extend_24b(uint32_t value) {
    if (value & 0x800000) // bit 23 = sign
        return static_cast<int32_t>(value | 0xFF000000); // extend sign
    return static_cast<int32_t>(value);
}


template <size_t RAM_SIZE = 0x10000, size_t PROGRAM_SIZE = 0xFFFF>
struct AsmDecoder {
    ObjectFile obj_file;
    std::unordered_map<std::string, Var> vars;
    size_t max_lines = PROGRAM_SIZE;
    std::vector<std::string> lines;
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
        if (tokens[0] == ".global" || tokens[0] == ".extern" || tokens[0] == ".entry") return ErrorCode::OK;

        InstrDef def = it->second;

        if (tokens.size() - 1 != def.args.size()) return ErrorCode::INVALID_ARG;

        std::array<uint8_t, 3> r{}; // rd, rs1, rs2
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

                    if (!obj_file.symbols.contains(label))
                        return ErrorCode::UNKNOWN_SYMBOL;

                    const Symbol& S = obj_file.symbols[label];

                    obj_file.relocations.push_back({Section::TEXT, static_cast<uint32_t>(cur_pc), RelocType::PC_REL_24, label });

                    if (S.bind == SymbolBinding::LOCAL && S.section == Section::TEXT) {
                        int32_t offset = static_cast<int32_t>(S.value) - static_cast<int32_t>(cur_pc + 1);
                        imm = sign_extend_24b(offset);
                    } else
                        imm = 0;

                    break;
                }
            case ArgType::VAR:
                {
                    const std::string& name = tokens[i + 1];
                    if (!obj_file.symbols.contains(name)) return ErrorCode::UNKNOWN_SYMBOL;

                    obj_file.relocations.push_back({
                        Section::TEXT,
                        static_cast<uint32_t>(cur_pc),
                        RelocType::ABS_32,
                        name
                    });

                    imm = 0; // linker will patch abs addr
                    break;
                }
            case ArgType::NONE:
                break;
            }
        }

        DecodedInstr result = DecodedInstr(def.opcode, r[0], r[1], r[2], imm);

        if (it != instr_table.end()) cur_pc++;
        obj_file.text.emplace_back(result);

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
            if (line == ".rodata") { cur_section = Section::RODATA; continue; }

            // labels
            if (line.ends_with(':')) {
                std::string name = line.substr(0, line.size() - 1);

                if (obj_file.symbols.contains(name) && obj_file.symbols[name].bind == SymbolBinding::LOCAL) return { ErrorCode::DUPLICATE_LABEL, i };


                uint32_t value = 0;

                switch (cur_section) {
                case Section::TEXT:
                    value = text_pc; break;
                case Section::DATA:
                    value = obj_file.data.size(); break;
                case Section::RODATA:
                    value = obj_file.rodata.size(); break;
                case Section::BSS:
                    value = obj_file.bss_size; break;
                default: break;
                }

                if (obj_file.symbols.contains(name)) {
                    obj_file.symbols[name].section = cur_section;
                    obj_file.symbols[name].value = value;
                }
                else
                    obj_file.symbols[name] = { name, cur_section, value, SymbolBinding::LOCAL };

                continue;
            }

            //variable / array declaration
            auto tokens = string_utils::slice_str(line, ' ');
            auto it = instr_table.find(tokens[0]);

            if (tokens[0] == ".global") {
                auto& S = obj_file.symbols[tokens[1]];
                S.name = tokens[1];
                S.bind = SymbolBinding::GLOBAL;
                continue;
            }
            if (tokens[0] == ".extern") {
                obj_file.symbols[tokens[1]] = {
                    tokens[1],
                    Section::NONE,
                    0,
                    SymbolBinding::EXTERN
                };
                continue;
            }
            if (tokens[0] == ".entry") {
                if (tokens.size() != 2)
                    return { ErrorCode::INVALID_ARG, i };

                obj_file.entry_symbol = tokens[1];
                continue;
            }

            if (it == instr_table.end()) {
                if (tokens[1] == "times" || tokens[1] == "TIMES") {
                    if (tokens.size() < 4) return { ErrorCode::INVALID_ARG, i };
                    auto [e_code_3, var_count] = parse_imm(tokens[2]);
                    auto [var_size, needs_init] = parse_D(tokens[3]);
                    if (e_code_3 != ErrorCode::OK || !var_size) return { ErrorCode::INVALID_ARG, i };
                    if (var_count < 1) return { ErrorCode::INVALID_ARG, i };

                    Var v(needs_init ? obj_file.data.size() : obj_file.bss_size, var_size, var_count);

                    if (needs_init) {
                        if (tokens.size() != 5) return { ErrorCode::INVALID_ARG, i };

                        auto [e_init, bytes] = parse_bytes(tokens[4], v.size);
                        if (e_init != ErrorCode::OK) return { e_init, i };

                        if (cur_section == Section::DATA)
                            obj_file.data.insert(obj_file.data.end(), bytes.begin(), bytes.end());
                        else if (cur_section == Section::RODATA)
                            obj_file.rodata.insert(obj_file.rodata.end(), bytes.begin(), bytes.end());
                        else
                            return { ErrorCode::VAR_OUTSIDE_R_SECTION, i };

                    }
                    else {
                        if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, i };
                        obj_file.bss_size += var_size;
                    }

                    obj_file.symbols[tokens[0]] = { tokens[0], cur_section, static_cast<uint32_t>(v.addr), SymbolBinding::LOCAL };
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

                Var v(needs_init ? obj_file.data.size() : obj_file.bss_size, var_size);
                if (needs_init) {
                    if (tokens.size() != 3) return { ErrorCode::INVALID_ARG, i };

                    auto [e_code_6, res] = parse_bytes(tokens[2], var_size);
                    if (e_code_6 != ErrorCode::OK) return { e_code_6, i };

                    if (cur_section == Section::DATA)
                        obj_file.data.insert(obj_file.data.end(), res.begin(), res.end());
                    else if (cur_section == Section::RODATA)
                        obj_file.rodata.insert(obj_file.rodata.end(), res.begin(), res.end());
                    else
                        return { ErrorCode::VAR_OUTSIDE_R_SECTION, i };
                }
                else {
                    if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, i };
                    obj_file.bss_size += var_size;
                }

                obj_file.symbols[tokens[0]] = { tokens[0], cur_section, static_cast<uint32_t>(v.addr), SymbolBinding::LOCAL };
                vars[tokens[0]] = v;
                continue;
            }

            // instructions only advance PC in .text
            if (cur_section == Section::TEXT)
                text_pc++;
        }

        return {};
    }

    std::pair<ObjectFile, ErrorInfo> decode(const std::string& asm_program) {
        obj_file = ObjectFile();
        vars.clear();
        cur_pc = 0;

        lines = string_utils::slice_str(asm_program, '\n');
        if (lines.size() > max_lines) return { obj_file, { ErrorCode::LINE_OVERFLOW, lines.size() - 1 } };

        auto e_info_f_pass = first_pass();
        if (e_info_f_pass.error_code != ErrorCode::OK)
            return { obj_file, e_info_f_pass };

        cur_section = Section::TEXT;

        if (lines.empty()) return {};
        if (lines.size() > max_lines) return { obj_file, { ErrorCode::LINE_OVERFLOW, lines.size() - 1 } };


        for (size_t i = 0; i < lines.size(); i++) {
            std::string cleaned_line = string_utils::normalize(lines[i]);
            if (cleaned_line.empty()) continue;

            ErrorCode e_code_text = decode_line(cleaned_line);
            if (e_code_text != ErrorCode::OK)
                return { obj_file, { e_code_text, i } };
        }

        if (obj_file.data.size() + obj_file.rodata.size() > RAM_SIZE) return { obj_file, { ErrorCode::RAM_OVERFLOW, 0 } };

        return {obj_file, {} };
    }
};


#endif