#ifndef ERGON_DATA_H
#define ERGON_DATA_H

#include <cstdint>
#include <string>


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
    PC_REL_32, // jumps, calls
    ABS_32 // data addresses
};

struct Relocation {
    Section section;
    uint32_t offset; // where to patch
    RelocType type;
    std::string symbol;
};

struct DecodedInstr {
    uint8_t opcode = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    int32_t imm = 0; // sign-extended imm / jump offset

    DecodedInstr() = default;

    DecodedInstr(uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, int32_t imm) : opcode(opcode), rd(rd), rs1(rs1), rs2(rs2), imm(imm) {}
};

#endif