#ifndef ERGON_DATA_H
#define ERGON_DATA_H

#include <cstdint>
#include <string>
#include <vector>


enum class Section {
    TEXT, // instructions
    DATA, // initialized data
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

struct Define {
    std::vector<std::string> parameters;
    std::string replacement;
};

struct Macro {
    std::vector<std::string> parameters;
    std::vector<std::string> body;
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
/*
* Instr:
* [0] opcode
* [1] rd
* [2] rs1
* [3] rs2
* [4..7] imm (32 bits, little-endian)
* format linker et BUS
*/
constexpr uint32_t INSTR_SIZE = 8;

inline void encode_instr(const DecodedInstr& instr, uint8_t* out) {
    out[0] = instr.opcode;
    out[1] = instr.rd;
    out[2] = instr.rs1;
    out[3] = instr.rs2;
    const auto imm_bits = static_cast<uint32_t>(instr.imm);
    out[4] = imm_bits & 0xFF;
    out[5] = (imm_bits >> 8) & 0xFF;
    out[6] = (imm_bits >> 16) & 0xFF;
    out[7] = (imm_bits >> 24) & 0xFF;
}

inline std::vector<uint8_t> to_bytes(const std::vector<DecodedInstr>& text) {
    std::vector<uint8_t> out(text.size() * INSTR_SIZE);
    for (size_t i = 0; i < text.size(); i++)
        encode_instr(text[i], &out[i * INSTR_SIZE]);
    return out;
}

#endif