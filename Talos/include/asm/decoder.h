#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "error.h"
#include "instructions.h"
#include "variables.h"
#include "parser.h"
#include "../computer/core.h"
#include "data.h"

#include <unordered_map>
#include <string>

/*
VOIR TUTOS sur www.tutorialspoint.com/assembly_programming

A FAIRE:

AJOUTER heap & stack: malloc & free instructions, (garbage collector (kind of) ?) (.heap & .stack sections)
FAIRE FPU
Super-instructions (merge addi+cmp+jl)
AJOUTER JIT
Profile-guided optimization (PGO) (maybe)
AJOUTER acès array avec: BYTE_TABLE[2] ou BYTE_TABLE + 2 ou BYTE_TABLE + 2 * 3 ...
AJOUTER constantes (assign, define)
AJOUTER un truc qui détecte les modifications de constantes (equ, assign, define)
AJOUTER solveur numérique pour les imm (ex: 0x3 + 0b11001 * (-133))
AJOUTER truc qui détecte les ram overflow lors des store et load !!
UPDATE le readme
AJOUTER les struct, offset, .asciz
*/


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
    std::unordered_map<std::string, int32_t> constants;
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
        if (line == ".section .text" || line == ".text") {
            cur_section = Section::TEXT;
            return { };
        }
        if (line == ".section .data" || line == ".data") {
            cur_section = Section::DATA;
            return { };
        }
        if (line == ".section .rodata" || line == ".rodata") {
            cur_section = Section::RODATA;
            return { };
        }
        if (line == ".section .bss" || line == ".bss") {
            cur_section = Section::BSS;
            return { };
        }

        //labels
        if (line.ends_with(':')) return { };

        const std::vector<std::string> tokens = string_utils::slice_str(line, ' ');
        const std::string& instr = tokens[0];
        auto it = instr_table.find(instr);


        if (cur_section != Section::TEXT) return { };
        if (instr == ".global" || instr == ".extern" || instr == ".entry") return { };

        InstrDef def = it->second;


        if (def.args.empty()) {
            DecodedInstr result = DecodedInstr(def.opcode, 0, 0, 0, 0);

            if (it != instr_table.end()) cur_pc++;
            obj_file.text.emplace_back(result);
            return { };
        }
        //trim instruction from line -> remove spaces (", " -> ",") -> slice into arguments
        const std::vector<std::string> args = string_utils::slice_str(string_utils::remove_char(line.substr(instr.size() + 1), ' '), ',');
        if (args.size() != def.args.size()) return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected " + std::to_string(def.args.size()) + " arguments" };

        std::array<uint8_t, 3> r{}; // rd, rs1, rs2
        int32_t imm = 0;

        for (size_t i = 0; i < def.args.size(); i++) {
            switch (def.args[i]) {
            case ArgType::REG:
                {
                    auto [e, temp] = parse_reg(args[i]);
                    if (e.code != ErrorCode::OK) return e;

                    r[def.args_pos[i]] = temp;
                    break;
                }
            case ArgType::IMM:
                {
                    auto [e, imm_val] = parse_imm(args[i], constants);
                    if (e.code != ErrorCode::OK) return e;

                    if (it->first == "movi") imm = imm_val;
                    else r[def.args_pos[i]] = imm_val;

                    break;
                }
            case ArgType::LABEL:
                {
                    const std::string& label = args[i];

                    if (!obj_file.symbols.contains(label)) return { ErrorCode::UNKNOWN_SYMBOL, "unknown symbol \"" + label + "\"" };

                    const Symbol& S = obj_file.symbols[label];

                    obj_file.relocations.push_back({Section::TEXT, static_cast<uint32_t>(cur_pc), RelocType::PC_REL_32, label });

                    if (S.bind == SymbolBinding::LOCAL && S.section == Section::TEXT) {
                        int32_t offset = static_cast<int32_t>(S.value) - static_cast<int32_t>(cur_pc + 1);
                        imm = offset;
                    } else
                        imm = 0;

                    break;
                }
            case ArgType::VAR:
                {
                    const std::string& name = args[i];
                    if (!obj_file.symbols.contains(name)) return { ErrorCode::UNKNOWN_SYMBOL, "unknown symbol \"" + name + "\"" };
                    if (obj_file.symbols[name].section == Section::RODATA)
                        if (check_if_constant(instr))
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
            if (line == ".section .text") { cur_section = Section::TEXT; continue; }
            if (line == ".section .data") { cur_section = Section::DATA; continue; }
            if (line == ".section .bss") { cur_section = Section::BSS; continue; }
            if (line == ".section .rodata") { cur_section = Section::RODATA; continue; }

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
            if (line[0] == '.') {
                ErrorInfo e = handle_directive(line, i);
                if (e.code != ErrorCode::OK) return e;
                continue;
            }

            // instructions only advance PC in .text
            if (cur_section == Section::TEXT)
                text_pc++;
        }

        return {};
    }

    size_t current_section_size() {
        switch (cur_section) {
        case Section::DATA:
            return obj_file.data.size();
        case Section::RODATA:
            return obj_file.rodata.size();
        case Section::BSS:
            return obj_file.bss_size;
        default: throw std::logic_error("unknown section");
        }
    }

    void modify_section_size(size_t new_size) {
        switch (cur_section) {
        case Section::DATA:
            obj_file.data.resize(new_size);
            break;
        case Section::RODATA:
            obj_file.rodata.resize(new_size);
            break;
        case Section::BSS:
            obj_file.bss_size = new_size;
            break;
        default: throw std::logic_error("unknown section");
        }
    }

    void align_section(size_t align) {
        size_t sz = current_section_size();
        size_t mask = align - 1;
        if (sz & mask)
            modify_section_size((sz + mask) & ~mask);
    }

    void emit_u8(uint8_t v) {
        auto& buf = (cur_section == Section::DATA) ? obj_file.data : obj_file.rodata;
        buf.push_back(v);
    }

    void emit_u16(uint16_t v) {
        emit_u8(v & 0xFF);
        emit_u8((v >> 8) & 0xFF);
    }

    void emit_u32(uint32_t v) {
        emit_u8(v & 0xFF);
        emit_u8((v >> 8) & 0xFF);
        emit_u8((v >> 16) & 0xFF);
        emit_u8((v >> 24) & 0xFF);
    }

    ErrorInfo handle_directive(const std::string& line, size_t i) {
        const auto tokens = string_utils::slice_str(line, ' ');
        const std::string& instr = tokens[0];
        const auto it = instr_table.find(instr);

        const std::vector<std::string> args = string_utils::slice_str(string_utils::remove_char(line.substr(instr.size() + 1), ' '), ',');

        if (instr == ".global") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 1", i };
            auto& S = obj_file.symbols[args[0]];
            S.name = args[0];
            S.bind = SymbolBinding::GLOBAL;
        }
        if (instr == ".extern") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 1", i };
            obj_file.symbols[args[0]] = {
                args[0],
                Section::NONE,
                0,
                SymbolBinding::EXTERN
            };
        }
        if (instr == ".entry") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 1", i };
            obj_file.entry_symbol = args[0];
        }

        if (instr == ".equ") {
            if (args.size() != 2)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 2", i };

            auto [e, value] = parse_imm(args[1], constants);
            if (e.code != ErrorCode::OK) return e;

            constants[args[0]] = value;
            return {};
        }
        if (instr == ".byte") {
            for (auto& a : args) {
                auto [e, v] = parse_imm(a, constants);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 1; // réserve 1 octet
                else
                    emit_u8(static_cast<uint8_t>(v));
            }
            return {};
        }
        if (instr == ".hword") {
            align_section(2);
            for (auto& a : args) {
                auto [e, v] = parse_imm(a, constants);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 2; // réserve 2 octets
                else
                    emit_u16(static_cast<uint16_t>(v));
            }
            return {};
        }
        if (instr == ".word") {
            align_section(4);
            for (auto& a : args) {
                auto [e, v] = parse_imm(a, constants);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 4; // réserve 4 octets
                else {
                    emit_u32(static_cast<uint32_t>(v));
                }
            }
            return {};
        }
        if (instr == ".space") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, ".space expects 1 arg", i };

            auto [e, size] = parse_imm(args[0], constants);
            if (e.code != ErrorCode::OK) return e;

            align_section(1);

            if (cur_section == Section::BSS)
                obj_file.bss_size += size;
            else {
                auto& buf = (cur_section == Section::DATA) ? obj_file.data : obj_file.rodata;
                buf.resize(buf.size() + size, 0);
            }
            return {};
        }

        if (instr == ".align") {
            auto [e, pow] = parse_imm(args[0], constants);
            if (e.code != ErrorCode::OK) return e;

            align_section(1u << pow);
            return {};
        }

        return { };
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