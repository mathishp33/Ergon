#ifndef ERGON_LINKER_H
#define ERGON_LINKER_H

#include <algorithm>

#include "assembler.h"
#include "error.h"

#include <string>


struct LinkedBinary {
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    std::vector<uint8_t> rodata;
    uint32_t bss_size = 0;

    uint32_t entry_pc = 0;
    uint32_t stack_size = 0;
};

struct GlobalSymbol {
    std::string name;
    Section section = Section::NONE;
    uint32_t value = 0;
    size_t obj_index = 0;
    GlobalSymbol() = default;
    GlobalSymbol(const Symbol& sym, size_t index) {
        name = sym.name;
        section = sym.section;
        value = sym.value;
        obj_index = index;
    }
};

inline std::pair<ErrorInfo, LinkedBinary> link(std::vector<ObjectFile>& objects, uint32_t base_address = 0) {
    LinkedBinary out;

    uint32_t total_data_size = 0;
    uint32_t total_rodata_size = 0;
    uint32_t total_text_instrs = 0;
    for (const auto& obj : objects) {
        total_data_size += static_cast<uint32_t>(obj.data.size());
        total_rodata_size += static_cast<uint32_t>(obj.rodata.size());
        total_text_instrs += static_cast<uint32_t>(obj.text.size());
    }

    const uint32_t text_bytes_total = total_text_instrs * INSTR_SIZE;

    uint32_t text_cursor = 0; // en unité d'Instr
    uint32_t data_cursor = text_bytes_total;
    uint32_t rodata_cursor = text_bytes_total + total_data_size;
    uint32_t bss_cursor  = text_bytes_total + total_data_size + total_rodata_size;

    bool entry_found = false;

    std::unordered_map<std::string, GlobalSymbol> globals;

    // assign bases + collect globals
    for (size_t i = 0; i < objects.size(); i++) {
        auto& obj = objects[i];

        obj.text_base = text_cursor;
        obj.data_base = data_cursor;
        obj.rodata_base = rodata_cursor;
        obj.bss_base = bss_cursor;

        for (auto& [name, sym] : obj.symbols) {
            if (sym.bind == SymbolBinding::GLOBAL) {
                if (globals.contains(name)) return { { ErrorCode::DUPLICATE_GLOBAL_SYMBOL, "duplicate global symbol" }, out };

                Symbol resolved = sym;
                switch (sym.section) {
                case Section::TEXT:
                    resolved.value += obj.text_base; break;
                case Section::DATA:
                    resolved.value += obj.data_base; break;
                case Section::RODATA:
                    resolved.value += obj.rodata_base; break;
                case Section::BSS:
                    resolved.value += obj.bss_base; break;
                default: break;
                }

                globals[name] = { resolved, i };
            }
        }
        text_cursor += obj.text.size();
        data_cursor += obj.data.size();
        rodata_cursor += obj.rodata.size();
        bss_cursor += obj.bss_size;
    }
    // pass 2
    for (auto& obj : objects)
        for (auto& [name, sym] : obj.symbols)
            if (sym.bind == SymbolBinding::EXTERN && !globals.contains(name))
                return { { ErrorCode::UNRESOLVED_EXTERN_SYMBOL, "unresolved extern symbol \"" + name + "\"" }, out };

    // merge sections
    for (auto& obj : objects) {
        out.text.insert(out.text.end(), obj.text.begin(), obj.text.end());
        out.data.insert(out.data.end(), obj.data.begin(), obj.data.end());
        out.rodata.insert(out.rodata.end(), obj.rodata.begin(), obj.rodata.end());
        out.bss_size += obj.bss_size;
    }

    // apply relocations
    for (auto& obj : objects) {
        if (!obj.entry_symbol.empty()) {
            if (!globals.contains(obj.entry_symbol))
                return { { ErrorCode::UNKNOWN_ENTRY_SYBOL, "unknown entry symbol \"" + obj.entry_symbol + "\"" }, out };

            out.entry_pc = globals[obj.entry_symbol].value * INSTR_SIZE + base_address;
            entry_found = true;

            if (obj.has_stack_size)
                out.stack_size = obj.stack_size;
        }
        else if (obj.has_stack_size) {
            return { { ErrorCode::STACK_SIZE_NOT_IN_ENTRY_FILE, "\".stack_size\" must be declared in the file containing \".entry\"" }, out };
        }

        for (auto& rel : obj.relocations) {
            uint32_t sym_addr = 0;
            if (!globals.contains(rel.symbol)) {
                const Symbol& S = obj.symbols[rel.symbol];

                switch (S.section) {
                case Section::TEXT:
                    sym_addr = (obj.text_base + S.value) * INSTR_SIZE;
                    break;

                case Section::DATA:
                    sym_addr = obj.data_base + S.value;
                    break;

                case Section::RODATA:
                    sym_addr = obj.rodata_base + S.value;
                    break;

                case Section::BSS:
                    sym_addr = obj.bss_base + S.value;
                    break;

                default:
                    break;
                }
            }
            else {
                const GlobalSymbol& GS = globals.at(rel.symbol);
                switch (GS.section) {
                case Section::TEXT:
                    sym_addr = GS.value * INSTR_SIZE; break;
                case Section::DATA:
                    sym_addr = GS.value; break;
                case Section::RODATA:
                    sym_addr = GS.value; break; // RAM offset = total data size
                case Section::BSS:
                    sym_addr = GS.value; break; // RAM offset = total data + rodata size
                default: break;
                }
            }

            DecodedInstr& I = out.text[obj.text_base + rel.offset];

            if (rel.type == RelocType::PC_REL_32) {
                auto pc = static_cast<int32_t>((obj.text_base + rel.offset) * INSTR_SIZE);
                I.imm = static_cast<int32_t>(sym_addr) - (pc);
            }
            if (rel.type == RelocType::ABS_32)
                I.imm = static_cast<int32_t>(sym_addr) + static_cast<int32_t>(base_address);

        }
    }

    //if (!entry_found) return { { ErrorCode::NO_ENTRY_DEFINED, "no entry defined, cannot know what should the starting PC" }, out };

    return { { }, out };
}



#endif