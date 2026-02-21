#ifndef ERGON_LINKER_H
#define ERGON_LINKER_H

#include "decoder.h"
#include "error.h"

#include <string>

/*

* ASM source //DONE
   ↓
Assembler (per file) //DONE
   ↓
ObjectFile (.o-like) //DONE
   ↓
Linker (multi-file, static libs) //PARTIALLY DONE
   ↓
Executable image (ELF-like)
   ↓
Loader
   ↓
Interpreter → Tier-2 JIT

 */

struct LinkedBinary {
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    std::vector<uint8_t> rodata;
    uint32_t bss_size = 0;

    uint32_t entry_pc = 0;
};

inline std::pair<LinkedBinary, ErrorCode> link(std::vector<ObjectFile>& objects) {
    LinkedBinary out;

    uint32_t text_cursor = 0;
    uint32_t data_cursor = 0;
    uint32_t rodata_cursor = 0;
    uint32_t bss_cursor  = 0;

    bool entry_found = false;

    std::unordered_map<std::string, Symbol> globals;

    // zssign bases + collect globals
    for (auto& obj : objects) {
        obj.text_base = text_cursor;
        obj.data_base = data_cursor;
        obj.rodata_base = rodata_cursor;
        obj.bss_base = bss_cursor;

        for (auto& [name, sym] : obj.symbols) {
            if (sym.bind == SymbolBinding::GLOBAL) {
                if (globals.contains(name)) return { out, ErrorCode::DUPLICATE_GLOBAL };

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

                globals[name] = resolved;
            }
        }
        text_cursor += obj.text.size();
        data_cursor += obj.data.size();
        rodata_cursor += obj.rodata.size();
        bss_cursor += obj.bss_size;
    }
    // pass 2
    for (auto& obj : objects) {
        for (auto& [name, sym] : obj.symbols) {
            if (sym.bind == SymbolBinding::EXTERN && !globals.contains(name)) {
                return { out, ErrorCode::UNRESOLVED_EXTERN };
            }
        }
    }

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
            if (!globals.contains(obj.entry_symbol)) return { out, ErrorCode::UNKNOWN_ENTRY };

            out.entry_pc = globals[obj.entry_symbol].value;
            entry_found = true;
        }

        for (auto& rel : obj.relocations) {
            Symbol S = obj.symbols.contains(rel.symbol) ? obj.symbols[rel.symbol] : globals.at(rel.symbol);

            //uint32_t sym_addr = (S.section == Section::TEXT) ? obj.text_base + S.value : (S.section == Section::DATA) ? obj.data_base + S.value : obj.bss_base + S.value;
            uint32_t sym_addr = 0;
            switch (S.section) {
                case Section::TEXT:
                    sym_addr = obj.text_base + S.value; break;
                case Section::DATA:
                    sym_addr = obj.data_base + S.value; break;
                case Section::RODATA:
                    sym_addr = obj.rodata_base + S.value; break;
                case Section::BSS:
                    sym_addr = obj.bss_base + S.value; break;
                default: break;
            }

            DecodedInstr& I = out.text[obj.text_base + rel.offset];

            if (rel.type == RelocType::PC_REL_24) {
                auto pc = static_cast<int32_t>(obj.text_base + rel.offset + 1);
                I.imm = sign_extend_24b(sym_addr - (pc - 1));
            }
            if (rel.type == RelocType::ABS_32) {
                I.imm = static_cast<int32_t>(sym_addr);
            }
        }
    }

    if (!entry_found) return { out, ErrorCode::NO_ENTRY_DEFINED };

    return { out, ErrorCode::OK };
}



#endif