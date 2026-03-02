#ifndef ERGON_LINKER_H
#define ERGON_LINKER_H

#include <algorithm>

#include "decoder.h"
#include "error.h"

#include <string>


struct LinkedBinary {
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    std::vector<uint8_t> rodata;
    uint32_t bss_size = 0;

    uint32_t entry_pc = 0;
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

inline std::pair<ErrorInfo, LinkedBinary> link(std::vector<ObjectFile>& objects) {
    LinkedBinary out;

    uint32_t text_cursor = 0;
    uint32_t data_cursor = 0;
    uint32_t rodata_cursor = 0;
    uint32_t bss_cursor  = 0;

    bool entry_found = false;

    std::unordered_map<std::string, GlobalSymbol> globals;

    // zssign bases + collect globals
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
            if (!globals.contains(obj.entry_symbol)) return { { ErrorCode::UNKNOWN_ENTRY_SYBOL, "unknown entry symbol \"" + obj.entry_symbol + "\"" }, out };

            out.entry_pc = globals[obj.entry_symbol].value;
            entry_found = true;
        }

        for (auto& rel : obj.relocations) {
            uint32_t sym_addr = 0;
            if (!globals.contains(rel.symbol)) {
                Symbol S = obj.symbols[rel.symbol];
                sym_addr = obj.text_base + S.value;
            }
            else {
                const GlobalSymbol& GS = globals.at(rel.symbol);
                switch (GS.section) {
                case Section::TEXT:
                    sym_addr = GS.value; break;
                case Section::DATA:
                    sym_addr = GS.value; break; //pas sur si c'est bon (voir ancien commit)
                case Section::RODATA:
                    sym_addr = GS.value; break; //pas sur si c'est bon (voir ancien commit)
                case Section::BSS:
                    sym_addr = GS.value; break; //pas sur si c'est bon (voir ancien commit)
                default: break;
                }
            }

            DecodedInstr& I = out.text[obj.text_base + rel.offset];

            if (rel.type == RelocType::PC_REL_32) {
                auto pc = static_cast<int32_t>(obj.text_base + rel.offset);
                I.imm = static_cast<int32_t>(sym_addr) - (pc + 1);
            }
            if (rel.type == RelocType::ABS_32)
                I.imm = static_cast<int32_t>(sym_addr);

        }
    }

    //if (!entry_found) return { { ErrorCode::NO_ENTRY_DEFINED, "no entry defined, cannot know what should the starting PC" }, out };

    return { { }, out };
}



#endif