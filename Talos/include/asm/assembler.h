#ifndef ERGON_ASM_INTERPRETER_H
#define ERGON_ASM_INTERPRETER_H

#include "error.h"
#include "instructions.h"
#include "variables.h"
#include "parser.h"
#include "data.h"

#include <unordered_map>
#include <string>
#include <functional>


/*
VOIR TUTOS sur www.tutorialspoint.com/assembly_programming

TODO:
CHANGER le sys_time dans environment_manager en 2038 !!!
UPDATE le readme
AJOUTER %if, %ifdef et %include
AJOUTER Dispatcher pour les system calls
rajouter open/close comme system calls ((open, close) -> donc memoire (SSD) persistant, spawn(thread))
AJOUTER kernel séparé
AJOUTER filesystem
AJOUTER scheduler
AJOUTER drivers
AJOUTER syscall handler (lower levels)

------------------------------------------------------------------------------------------------------------------------

① Storage virtuel brut
        ↓
② DISK_READ / DISK_WRITE
        ↓
③ format de disque minimal
        ↓
④ filesystem minimal
        ↓
⑤ fichiers
        ↓
⑥ format exécutable Ergon
        ↓
⑦ loader
        ↓
⑧ processus
        ↓
⑨ scheduler
        ↓
⑩ SLEEP
        ↓
⑪ séparation USER/KERNEL
        ↓
⑫ EXEC / EXIT / WAIT

------------------------------------------------------------------------------------------------------------------------

┌─────────────────────────────┐
│ Kernel                      │
│                             │
│ boot                        │
│ memory manager              │
│ process manager             │
│ scheduler                   │
│ syscall handler             │
│ drivers                     │
│ filesystem                  │
│ executable loader           │
└─────────────────────────────┘

------------------------------------------------------------------------------------------------------------------------

Programme Ergon
      │
      │ SYSCALL
      ▼
   CPU / VM
      │
      │ trap
      ▼
Kernel Ergon
      │
      │ traite le syscall
      ▼
driver / filesystem / scheduler

------------------------------------------------------------------------------------------------------------------------

USER program
     │
     │ SYSCALL
     ▼
┌──────────────┐
│ CPU          │
│              │
│ mode USER    │
└──────┬───────┘
       │
       │ trap
       ▼
┌──────────────┐
│ Kernel       │
│              │
│ mode KERNEL  │
└──────────────┘

ATTENTION AUX MECANISMES DE TRAP !!!!!
SEUL LE KERNEL DOIT POSSEDER LES DRIVERS


Executable loader
       ↓
Filesystem
       ↓
Storage driver
       ↓
StorageDevice


USER PROGRAM
     │
     │ open/read/exec
     ▼
KERNEL
├── syscall handler
├── process manager
├── filesystem
├── executable loader
├── storage driver
└─────────────┐
              ▼
        VM Storage Device
              │
              ▼
       persistent disk

------------------------------------------------------------------------------------------------------------------------

VM
│
├── ROM
│    └── boot firmware
│
├── RAM
│
└── Storage
     └── kernel executable

------------------------------------------------------------------------------------------------------------------------

RESET
 ↓
ROM boot code
 ↓
initialisation hardware
 ↓
initialisation RAM
 ↓
initialisation storage
 ↓
charger kernel depuis storage
 ↓
kernel → RAM
 ↓
jump kernel entry

------------------------------------------------------------------------------------------------------------------------

┌────────────────────────────────────┐
│          USER PROGRAMS             │
│                                    │
│ shell / applications / games       │
└────────────────┬───────────────────┘
                 │
              SYSCALL
                 │
                 ▼
┌────────────────────────────────────┐
│              KERNEL                │
│                                    │
│ syscall handler                    │
│ scheduler                          │
│ process manager                    │
│ memory manager                     │
│ filesystem                         │
│ drivers                            │
│ executable loader                  │
└────────────────┬───────────────────┘
                 │
           hardware access
                 │
                 ▼
┌────────────────────────────────────┐
│                VM                  │
│                                    │
│ CPU                                │
│ RAM                                │
│ ROM                                │
│ Storage                            │
│ timers / devices                   │
└────────────────────────────────────┘


*/


struct ObjectFile {
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    std::vector<uint8_t> rodata;
    uint32_t bss_size = 0;

    uint32_t text_base = 0;
    uint32_t data_base = 0;
    uint32_t rodata_base = 0;
    uint32_t bss_base = 0;

    std::unordered_map<std::string, Symbol> symbols;
    std::vector<Relocation> relocations;

    std::string entry_symbol;

    bool has_stack_size = false;
    uint32_t stack_size = 0;
};


struct RecursionHandler {
    size_t rep_depth = 0;
    size_t macro_expansion = 0;

    bool handle() {
        rep_depth++;
        macro_expansion++;
        return rep_depth > 100 || macro_expansion > 100;
    }
};

struct PreProcesser {
    std::unordered_map<std::string, Value> constants; // %equ
    std::unordered_map<std::string, Value> variables; // %assign
    std::unordered_map<std::string, Define> defines; // %define
    std::unordered_map<std::string, Macro> macros; // %macro

    static std::string delimit_string(const std::string& s, size_t& idx, const std::function<bool(char, size_t)>& condition ) {
        size_t start = idx;
        for (; idx < s.size() && condition(s[idx], idx); idx++) {}
        return s.substr(start, idx - start);
    }

    static std::string leading_identifier(const std::string& s) {
        size_t idx = 0;
        return delimit_string(s, idx, [](char c, size_t) {
            return std::isalnum((unsigned char)c) || c == '_';
        });
    }

    static std::pair<ErrorInfo, std::string> find_block_end(const std::vector<std::string>& lines, size_t& idx, const std::string& begin_dir,
        const std::string& end_dir, ErrorInfo e_i) {
        size_t depth = 1;
        std::string out;
        for (; idx < lines.size(); idx++) {
            std::string local_line = string_utils::normalize(lines[idx]);
            if (local_line.starts_with(begin_dir))
                depth++;
            else if (local_line == end_dir) {
                depth--;
                if (depth == 0)
                    break;
            }
            out += lines[idx] + "\n";
        }
        e_i.index_line = idx;
        if (depth != 0)
            return { e_i, "" };
        return { { }, out };
    }

    ErrorInfo preprocess(std::string& file, RecursionHandler RH = RecursionHandler()) {
        std::vector<std::string> lines = string_utils::slice_str(file, '\n');
        if (RH.handle())
            return { ErrorCode::PREPROC_RECURSION, "infinite recursion in the preprocessor", 0 };
        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = string_utils::normalize(lines[i]);

            if (line.empty()) continue;

            //handle defines
            for (const auto& [name, define] : defines) {
                if (define.parameters.empty()) {
                    std::string line_copy = line;
                    string_utils::replace_string_as_token(line_copy, name, define.replacement);
                    lines[i] = line_copy;
                }
                else {
                    if (leading_identifier(line) == name) {
                        std::string line_copy = line;
                        if (string_utils::rep_counter(line, ')') != 1)
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        if (string_utils::rep_counter(line, '(') != 1)
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        if (line[name.size()] == '(') {
                            size_t idx = name.size() + 1;
                            std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                            std::vector<std::string> args = string_utils::slice_str(compacted_args, ',');
                            for (auto& arg : args)
                                arg = string_utils::remove_char(arg, ' ');
                            string_utils::replace_string_as_token(line_copy, line, define.replacement);
                            if (define.parameters.size() != args.size())
                                return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " + std::to_string(define.parameters.size()), i };
                            for (size_t j = 0; j < args.size(); j++)
                                string_utils::replace_string_as_token(line_copy, define.parameters[j], args[j]);
                            lines[i] = line_copy;
                        }
                        else {
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        }

                    }
                }
            }
            line = string_utils::normalize(lines[i]);
            for (const auto& [name, macro] : macros) {
                if (leading_identifier(line) == name) {
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (line[name.size()] == '(') {
                        size_t idx = name.size() + 1;
                        std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                        std::vector<std::string> args = string_utils::slice_str(compacted_args, ',');
                        for (auto& arg : args)
                            arg = string_utils::remove_char(arg, ' ');
                        if (macro.parameters.size() != args.size())
                            return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " + std::to_string(macro.parameters.size()), i };
                        std::vector<std::string> body;
                        for (const auto& local_line : macro.body) {
                            std::string local_line_copy = local_line;
                            for (size_t j = 0; j < args.size(); j++)
                                string_utils::replace_string_as_token(local_line_copy, macro.parameters[j], args[j]);
                            body.emplace_back(local_line_copy);
                        }
                        lines[i].clear();
                        lines.insert(lines.begin() + i, body.begin(), body.end());
                        //i += body.size();
                        i--;
                        line = string_utils::normalize(lines[i]);
                    }
                    else {
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    }
                }
            }


            if (!line.starts_with("%")) continue;

            size_t idx = 0;
            std::string instr = delimit_string(line, idx, [](char c, size_t idx) { return c != ' '; });
            size_t instr_idx = idx++;

            size_t name_idx = instr_idx;
            std::string name;
            if (instr != "%rep") {
                name = delimit_string(line, idx, [](char c, size_t idx) { return c != ' ' && c != '('; });
                name_idx = idx++;
                if (!string_utils::check_cst_name(name))
                    return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", i };
            }

            if (instr == "%equ") {
                std::string expr = line.substr(name_idx);
                auto [e, value] = parse_expr(expr, constants, variables);
                if (e.code != ErrorCode::OK) return e;

                if (constants.contains(name))
                    return { ErrorCode::DUPLICATE_CONSTANT, "duplicate constant \"" + name + "\"", i };
                constants[name] = value;
                lines[i].clear();
            }
            if (instr == "%assign") {
                std::string expr = line.substr(name_idx);
                auto [e, value] = parse_expr(expr, constants, variables);
                if (e.code != ErrorCode::OK) return e;

                variables[name] = value;
                lines[i].clear();
            }
            if (instr == "%define") {
                std::vector<std::string> args;
                std::string replacement;
                if (line[idx - 1] == '(') { //with args
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };

                    std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                    args = string_utils::slice_str(compacted_args, ',');
                    for (auto& arg : args)
                        arg = string_utils::remove_char(arg, ' ');
                    replacement = line.substr(++idx);
                }
                else { //no args
                    replacement = line.substr(idx);
                }

                if (defines.contains(name))
                    return { ErrorCode::DUPLICATE_DEFINE, "duplicate define \"" + name + "\"", i };
                defines[name] = { args, replacement };
                lines[i].clear();
            }
            if (instr == "%macro") {
                std::vector<std::string> args;
                std::vector<std::string> body;
                if (line[idx - 1] == '(') { //with args
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };

                    std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                    args = string_utils::slice_str(compacted_args, ',');
                    for (auto& arg : args)
                        arg = string_utils::remove_char(arg, ' ');

                    size_t j = i + 1;
                    for (; ; j++) {
                        if (j > lines.size())
                            return { ErrorCode::MISSING_ENDMACRO, "missing an %endmacro", i };

                        if (string_utils::normalize(lines[j]).starts_with("%endmacro"))
                            break;
                        body.emplace_back(lines[j]);
                    }
                    for (size_t k = i; k < j + 1; k++)
                        lines[k].clear();
                    i = j;
                }
                else {
                    return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                }

                if (macros.contains(name))
                    return { ErrorCode::DUPLICATE_MACRO, "duplicate macro \"" + name + "\"", i };
                macros[name] = { args, body };
            }
            if (instr == "%rep") {
                std::string expr = line.substr(name_idx);
                auto [e_0, value] = parse_expr(expr, constants, variables);
                if (e_0.code != ErrorCode::OK) return e_0;

                size_t j = i + 1;
                auto [e_1, body] = find_block_end(lines, j, "%rep ", "%endrep",
                    {ErrorCode::MISSING_ENDREP, "missing an %endrep", i });
                if (e_1.code != ErrorCode::OK) return e_1;

                std::vector<std::string> to_add;
                for (int k = 0; k < value; k++) {
                    std::string body_copy = body;
                    ErrorInfo e_i = preprocess(body_copy, RH);
                    if (e_i.code != ErrorCode::OK) {
                        e_i.index_line += i;
                        return e_i;
                    }
                    for (const auto& local_line : string_utils::slice_str(body_copy, '\n'))
                        to_add.push_back(local_line);
                }
                for (size_t k = i; k < j + 1; k++)
                    lines[k].clear();
                i = j + 1;
                lines.insert(lines.begin() + i, to_add.begin(), to_add.end());
                i += to_add.size();
                i--;
            }
            if (instr == "%if") {

            }
            if (instr == "%ifdef") {

            }


        }
        file.clear();
        for (const auto& line : lines)
            if (!line.empty())
                file += line + "\n";

        return { };
    }

    // a rajouter
    // %if
    // %ifdef

};



struct Assembler {
    PreProcesser preproc;
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
        if (instr == ".global" || instr == ".extern" || instr == ".entry" || instr == ".stack_size") return { };

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
                    auto [e, imm_val] = parse_expr(args[i], preproc.constants, preproc.variables);
                    if (e.code != ErrorCode::OK) return e;

                    if (disp8_ops.contains(def.opcode)) {
                        if (imm_val < -128 || imm_val > 127)
                            return { ErrorCode::INVALID_IMM, "wrong imm interval \"" + it->first + "\"" };
                        r[def.args_pos[i]] = static_cast<uint8_t>(imm_val);
                    } else {
                        imm = imm_val;
                    }
                    break;
                }
            case ArgType::LABEL:
                {
                    const std::string& label = args[i];

                    if (!obj_file.symbols.contains(label)) return { ErrorCode::UNKNOWN_SYMBOL, "unknown symbol \"" + label + "\"" };

                    const Symbol& S = obj_file.symbols[label];

                    obj_file.relocations.push_back({Section::TEXT, static_cast<uint32_t>(cur_pc), RelocType::PC_REL_32, label });

                    if (S.bind == SymbolBinding::LOCAL && S.section == Section::TEXT) {
                        int32_t offset = static_cast<int32_t>(S.value) - static_cast<int32_t>(cur_pc);
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
                        if (var_is_constant(instr))
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

    size_t current_section_size() const {
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
        if (instr == ".stack_size") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, "expected 1 argument", i };
            if (obj_file.has_stack_size)
                return { ErrorCode::DUPLICATE_STACK_SIZE, ".stack_size defined multiple times in this file", i };

            auto [e, v] = parse_expr(args[0], preproc.constants, preproc.variables);
            if (e.code != ErrorCode::OK) return e;

            obj_file.stack_size = static_cast<uint32_t>(v);
            obj_file.has_stack_size = true;
            return { };
        }
        if (instr == ".byte") {
            for (auto& a : args) {
                auto [e, v] = parse_expr(a, preproc.constants, preproc.variables);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 1; // réserve 1 octet
                else
                    emit_u8(static_cast<uint8_t>(v));
            }
            return { };
        }
        if (instr == ".hword") {
            for (auto& a : args) {
                auto [e, v] = parse_expr(a, preproc.constants, preproc.variables);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 2; // réserve 2 octets
                else
                    emit_u16(static_cast<uint16_t>(v));
            }
            return { };
        }
        if (instr == ".word") {
            for (auto& a : args) {
                auto [e, v] = parse_expr(a, preproc.constants, preproc.variables);
                if (e.code != ErrorCode::OK) return e;

                if (cur_section == Section::BSS)
                    obj_file.bss_size += 4; // réserve 4 octets
                else {
                    emit_u32(static_cast<uint32_t>(v));
                }
            }
            return { };
        }
        if (instr == ".zero") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 1", i };
            auto [e, v] = parse_expr(args[0], preproc.constants, preproc.variables);
            if (e.code != ErrorCode::OK) return e;

            if (cur_section == Section::BSS)
                obj_file.bss_size += v; // réserve 1 octet
            else
                for (int _ = 0; _ < v; _++)
                    emit_u8(0);

            return { };
        }
        if (instr == ".asciz") {
            std::string expr = line.substr(instr.size() + 1);
            if (string_utils::rep_counter(expr, '\"') != 2)
                return { ErrorCode::INVALID_ARG_SIZE, "invalid argument size, expected 2", i };
            bool quote_encountered = false;
            for (size_t i = 1; i < expr.size() - 1; i++) {
                emit_u8(expr[i]);
            }
            emit_u8('\0');

        }
        if (instr == ".space") {
            if (args.size() != 1)
                return { ErrorCode::INVALID_ARG_SIZE, ".space expects 1 arg", i };

            auto [e, size] = parse_expr(args[0], preproc.constants, preproc.variables);
            if (e.code != ErrorCode::OK) return e;

            align_section(1);

            if (cur_section == Section::BSS)
                obj_file.bss_size += size;
            else {
                auto& buf = (cur_section == Section::DATA) ? obj_file.data : obj_file.rodata;
                buf.resize(buf.size() + size, 0);
            }
            return { };
        }

        if (instr == ".align") {
            auto [e, pow] = parse_expr(args[0], preproc.constants, preproc.variables);
            if (e.code != ErrorCode::OK) return e;

            align_section(1u << pow);
            return { };
        }

        return { };
    }

    std::pair<ObjectFile, ErrorInfo> decode(const std::string& file) {
        obj_file = ObjectFile();
        vars.clear();
        cur_pc = 0;

        preproc = PreProcesser();
        std::string copy_file = file;
        ErrorInfo e_pp = preproc.preprocess(copy_file);
        if (e_pp.code != ErrorCode::OK) return { obj_file, e_pp };

        lines = string_utils::slice_str(copy_file, '\n');
        if (lines.empty()) return { };

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
        return { obj_file, { } };
    }
};


#endif