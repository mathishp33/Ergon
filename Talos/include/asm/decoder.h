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
Super-instructions (merge addi+cmp+jl)
AJOUTER JIT
Profile-guided optimization (PGO) (maybe)
AJOUTER acès array avec: BYTE_TABLE[2] ou BYTE_TABLE + 2 ou BYTE_TABLE + 2 * 3 ...
AJOUTER constantes (equ, assign, define)
AJOUTER un truc qui détecte les modifications de constantes (rodata, equ, assign, define)
AJOUTER solveur numérique pour les imm (ex: 0x3 + 0b11001 * (-133))
AJOUTER truc qui détecte les ram overflow lors des store et load !!
//UPDATE le readme
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
    Section section = Section::NONE;
    uint32_t value = 0; // offset in section
    SymbolBinding bind = SymbolBinding::LOCAL;
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

//true if not good
inline bool check_if_constant(const std::string& instr) {
    if (instr == "stb") return true;
    if (instr == "sth") return true;
    if (instr == "stw") return true;
    return false;
}

struct AsmDecoder {
    ObjectFile obj_file;
    std::unordered_map<std::string, Var> vars;
    std::vector<std::string> lines;
    Section cur_section = Section::TEXT;
    size_t cur_pc = 0;

    int get_var_addr(const std::string& var) {
        auto it = vars.find(var);
        if (it != vars.end())
            return static_cast<int>(it->second.addr);
        return -1;
    }

    ErrorInfo decode_line(const std::string& line) {
        //sections
        if (line == "section .text" || line == ".text") {
            cur_section = Section::TEXT;
            return { };
        }
        if (line == "section .data" || line == ".data") {
            cur_section = Section::DATA;
            return { };
        }
        if (line == "section .rodata" || line == ".rodata") {
            cur_section = Section::RODATA;
            return { };
        }
        if (line == "section .bss" || line == ".bss") {
            cur_section = Section::BSS;
            return { };
        }

        //labels
        if (line.ends_with(':')) return { };

        const std::vector<std::string> tokens = string_utils::slice_str(line, ' ');

        auto it = instr_table.find(tokens[0]);

        if (cur_section != Section::TEXT) return { };
        if (tokens[0] == ".global" || tokens[0] == ".extern" || tokens[0] == ".entry") return { };

        InstrDef def = it->second;

        if (tokens.size() - 1 != def.args.size()) return { };

        std::array<uint8_t, 3> r{}; // rd, rs1, rs2
        int32_t imm = 0;

        for (size_t i = 0; i < def.args.size(); i++) {
            switch (def.args[i]) {
            case ArgType::REG:
                {
                    auto [e, temp] = parse_reg(tokens[i + 1]);
                    if (e.code != ErrorCode::OK) return e;

                    r[def.args_pos[i]] = temp;
                    break;
                }
            case ArgType::IMM:
                {
                    auto [e, imm_val] = parse_imm(tokens[i + 1]);
                    if (e.code != ErrorCode::OK) return e;

                    if (it->first == "movi") imm = sign_extend_24b(imm_val);
                    else r[def.args_pos[i]] = imm_val;

                    break;
                }
            case ArgType::LABEL:
                {
                    const std::string& label = tokens[i + 1];

                    if (!obj_file.symbols.contains(label)) return { ErrorCode::UNKNOWN_SYMBOL, "unknown symbol \"" + label + "\"" };

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
                    if (!obj_file.symbols.contains(name)) return { ErrorCode::UNKNOWN_SYMBOL, "unknown symbol \"" + name + "\"" };
                    if (obj_file.symbols[name].section == Section::RODATA)
                        if (check_if_constant(tokens[0]))
                            return { ErrorCode::RODATA_VAR_MODIFIED, "rodata variable \"" + name + "\" is being modified" };

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

        return { };
    }

    ErrorInfo first_pass() {
        cur_section = Section::TEXT;
        size_t text_pc = 0;

        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = string_utils::normalize(lines[i]);
            if (line.empty()) continue;

            // sections
            if (line == "section .text") { cur_section = Section::TEXT; continue; }
            if (line == "section .data") { cur_section = Section::DATA; continue; }
            if (line == "section .bss") { cur_section = Section::BSS; continue; }
            if (line == "section .rodata") { cur_section = Section::RODATA; continue; }

            // labels
            if (line.ends_with(':')) {
                std::string name = line.substr(0, line.size() - 1);

                if (obj_file.symbols.contains(name) && obj_file.symbols[name].bind == SymbolBinding::LOCAL) return { ErrorCode::DUPLICATE_LABEL, "duplicate label \"" + name + "\"", i };


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
                    return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 1", i };

                obj_file.entry_symbol = tokens[1];
                continue;
            }

            if (it == instr_table.end()) {
                if (tokens[1] == "times" || tokens[1] == "TIMES") {
                    if (tokens.size() < 4) return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected more than 3", i };
                    auto [e_0, var_count] = parse_imm(tokens[2]);
                    if (e_0.code != ErrorCode::OK) return { e_0.code, e_0.message, i };
                    auto [var_size, needs_init] = parse_D(tokens[3]);
                    if (!var_size) return { ErrorCode::INVALID_ARG, "invalid argument, expected a declaration directive or a reserve directive", i };
                    if (var_count < 1) return { ErrorCode::INVALID_ARG, i };

                    Var v(needs_init ? obj_file.data.size() : obj_file.bss_size, var_size, var_count);

                    if (needs_init) {
                        if (tokens.size() != 5) return { ErrorCode::INVALID_ARG, i };
                        auto [e_1, bytes] = parse_bytes(tokens[4], v.size);
                        if (e_1.code != ErrorCode::OK) return { e_1.code, e_1.message, i };

                        if (cur_section == Section::DATA)
                            obj_file.data.insert(obj_file.data.end(), bytes.begin(), bytes.end());
                        else if (cur_section == Section::RODATA)
                            obj_file.rodata.insert(obj_file.rodata.end(), bytes.begin(), bytes.end());
                        else
                            return { ErrorCode::VAR_OUTSIDE_RIGHT_SECTION, "variable \""+ tokens[0] + " \" is outside data or rodata section", i };
                    }
                    else {
                        if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, "variable \""+ tokens[0] + " \" is outside bss section", i };
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

                    auto [e, res] = parse_bytes(tokens[2], var_size);
                    if (e.code != ErrorCode::OK) return { e.code, e.message, i };

                    if (cur_section == Section::DATA)
                        obj_file.data.insert(obj_file.data.end(), res.begin(), res.end());
                    else if (cur_section == Section::RODATA)
                        obj_file.rodata.insert(obj_file.rodata.end(), res.begin(), res.end());
                    else
                        return { ErrorCode::VAR_OUTSIDE_RIGHT_SECTION, "variable \""+ tokens[0] + " \" is outside data or rodata section", i };
                }
                else {
                    if (cur_section != Section::BSS) return { ErrorCode::VAR_OUTSIDE_BSS, "variable \""+ tokens[0] + " \" is outside bss section", i };
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
        if (lines.empty()) return {};

        auto e_fp = first_pass();
        if (e_fp.code != ErrorCode::OK) return { obj_file, e_fp };

        cur_section = Section::TEXT;
        for (size_t i = 0; i < lines.size(); i++) {
            std::string cleaned_line = string_utils::normalize(lines[i]);
            if (cleaned_line.empty()) continue;

            ErrorInfo e = decode_line(cleaned_line);
            e.index_line = i;
            if (e.code != ErrorCode::OK) return { obj_file, e };
        }
        return {obj_file, {} };
    }
};


#endif