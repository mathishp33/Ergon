#ifndef ERGON_DECODER_H
#define ERGON_DECODER_H

#include <unordered_map>

enum class Section {
    TEXT, // instructions
    DATA, // initialized data
    //HEAP, //
    //STACK, //
    //RODATA, // constants (optional)
    BSS, // un-initialized data
};

enum class SymbolBinding {
    LOCAL,
    GLOBAL,
    EXTERN
};

struct Symbol {
    std::string name;
    Section section;
    uint32_t value; // offset in section
    SymbolBinding bind;
};

struct Label {
    Section section;
    size_t value;
};

struct ObjectFile {
    //sections
    std::vector<DecodedInstr> text;
    std::vector<uint8_t> data;
    size_t bss_size = 0;
    //symbols
    std::unordered_map<std::string, Label> labels;
};


static int32_t sign_extend_24b(uint32_t value) {
    if (value & 0x800000) // bit 23 = sign
        return static_cast<int32_t>(value | 0xFF000000); // extend sign
    return static_cast<int32_t>(value);
}

#endif