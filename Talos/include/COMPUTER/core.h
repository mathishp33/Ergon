#ifndef ERGON_CORE_H
#define ERGON_CORE_H

#include "alu.h"

#include <cstdint>
#include <vector>
#include <array>


enum OPCODE : uint8_t {
    //----------------- ALU OPERATIONS -----------------
    ADD  , // R-type: add rd, rs1, rs2
    SUB  , // R-type: sub rd, rs1, rs2
    MUL  , // R-type: mul rd, rs1, rs2
    DIV  , // R-type: div rd, rs1, rs2
    MOD  , // R-type: mod rd, rs1, rs2
    ADDI , // I-type: addi rd, rs, imm
    SUBI , // I-type: subi rd, rs, imm
    MULI , // I-type: muli rd, rs, imm
    DIVI , // I-type: divi rd, rs, imm
    MODI , // I-type: modi rd, rs, imm

    AND  , // R-type: and rd, rs1, rs2
    OR   , // R-type: or rd, rs1, rs2
    XOR  , // R-type: xor rd, rs1, rs2
    ANDI , // I-type: andi rd, rs, imm
    ORI  , // I-type: ori rd, rs, imm
    XORI , // I-type: xori rd, rs, imm

    SHL  , // R-type: shl rd, rs, rt
    SHR  , // R-type: shr rd, rs, rt
    SAR  , // R-type: sar rd, rs, rt
    ROL  , // R-type: rol rd, rs, rt
    ROR  , // R-type: ror rd, rs, rt
    SHLI , // I-type: shli rd, rs, imm
    SHRI , // I-type: shri rd, rs, imm
    SARI , // I-type: sari rd, rs, imm
    ROLI , // I-type: roli rd, rs, imm
    RORI , // I-type: rori rd, rs, imm

    CMP  , // R-type: cmp rs1, rs2
    CMPU , // R-type: cmpu rs1, rs2
    TEST , // R-type: test rs1, rs2
    CMPI , // I-type: cmpi rs, imm
    CMPUI, // I-type: cmpui rs, imm (unsigned)
    TESTI, // I-type: testI rs, imm

    INC , // J-type: inc rd
    DEC , // J-type: dec rd
    NOT , // J-type: not rd, rs
    ABS , // J-type: abs rd, rs
    NEG , // J-type: neg rd, rs
    MIN , // R-type: min rd, rs1, rs2
    MAX , // R-type: max rd, rs1, rs2
    MINI, // I-type: mini rd, rs1, imm
    MAXI, // I-type: maxi rd, rs1, imm

    //----------------- FPU OPERATIONS -----------------

    AFAIRE0,
    AFAIRE1,
    AFAIRE2,
    AFAIRE3,
    AFAIRE4,
    AFAIRE5,
    AFAIRE6,

    //----------------- MEMORY OPERATIONS -----------------
    MOV_IMM , // mov rd, imm
    MOV_REG , // mov rd, rs1
    LDB_ABS , // load byte rd, [imm]
    LDH_ABS , // load half-word rd, [imm] (16b)
    LDW_ABS , // load word rd, [imm] (16b)
    STB_ABS , // store byte rd, [imm] (16b)
    STH_ABS , // store half-word rd, [imm] (16b)
    STW_ABS , // store word rd, [imm] (16b)
    LDB_BASE, // load byte rd, [rs1 + imm]
    LDH_BASE, // load half-word rd, [rs1 + imm]
    LDW_BASE, // load word rd, [rs1 + imm]
    STB_BASE, // store byte rd, [rs1 + imm]
    STH_BASE, // store half-word rd, [rs1 + imm]
    STW_BASE, // store word rd, [rs1 + imm]
    PUSH    , // push rs1
    POP     , // pop rd
    LEA     , // lea rd, rs1, imm
    SWAP    , // swap rd, rs1
    CLR     , // clr rd
    MEMCPY  , // memcpy rd, rs1, imm (length = imm)

    //----------------- PROGRAM OPERATIONS -----------------
    JMP , // jmp label (24b) (RELATIVE JUMP)
    JZ  , // jz label (24b) (JMPR if Z flag is true)
    JNZ , // jnz label (24b) (JMPR if Z flag is false)
    JG  , // jg label (24b) (JMPR if both Z and N flag are false)
    JL  , // jl label (24b) (JMPR if N flag is true)
    CALL, // call label (24b)  (JMPR and saves the current PC)
    RET , // ret (load previous PC and JMPR there)

    HALT // halt (stops program)
};

struct DecodedInstr {
    uint8_t opcode;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    int32_t imm;     // sign-extended immediate / jump offset
};

struct SimpleCore {
    std::array<uint32_t, 16> regs{};
    uint32_t& SP = regs[14]; // Stack Pointer
    uint32_t PC = 0; // Program Counter
    Flags flags;
    bool inc_pc = true;

    std::vector<uint8_t>& ram;

    ALU alu;

    SimpleCore(std::vector<uint8_t>& ram) : ram(ram) {
        SP = ram.size() - 1;
    }

    void R_type_instr(ALUOp op, uint8_t rd, uint8_t rs1, uint8_t rs2) {
        auto r = ALU::exec(op, regs[rs1], regs[rs2]);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }
    void I_type_instr(ALUOp op, uint8_t rd, uint8_t rs, uint8_t imm) {
        int32_t imm32 = static_cast<int8_t>(imm);
        auto r = alu.exec(op, regs[rs], imm32);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }
    void J_type_instr(ALUOp op, uint8_t rd, uint8_t rs) {
        auto r = ALU::exec(op, regs[rs], regs[rs]);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }
    void J_type_instr(ALUOp op, uint8_t rd) {
        auto r = ALU::exec(op, regs[rd], 0);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }

    // <- memory[addr]
    uint32_t load32(uint32_t addr) {
        if (addr + 3 >= ram.size()) return 0;
        return ram[addr] |
               (ram[addr + 1] << 8) |
               (ram[addr + 2] << 16) |
               (ram[addr + 3] << 24);
    }
    uint16_t load16(uint32_t addr) {
        if (addr + 1 >= ram.size()) return 0;
        return ram[addr] |
               (ram[addr + 1] << 8);
    }
    uint8_t load8(uint32_t addr) {
        if (addr >= ram.size()) return 0;
        return ram[addr];
    }
    // value -> ram
    void store32(uint32_t addr, uint32_t value) {
        if (addr + 3 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
        ram[addr + 2] = (value >> 16) & 0xFF;
        ram[addr + 3] = (value >> 24) & 0xFF;
    }
    void store16(uint32_t addr, uint16_t value) {
        if (addr + 1 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
    }
    void store8(uint32_t addr, uint8_t value) {
        if (addr >= ram.size()) return;
        ram[addr] = value;
    }

    // void update_flags(uint8_t r, bool C, bool V) {
    //     flags.Z = (regs[r] == 0);
    //     flags.N = (regs[r] < 0);
    //     flags.C = C;
    //     flags.V = V;
    // }
    // void update_flags(uint8_t r, bool V) {
    //     flags.Z = (regs[r] == 0);
    //     flags.N = (regs[r] < 0);
    //     flags.V = V;
    // }
    void update_flags(uint8_t r) {
        flags.Z = (regs[r] == 0);
        flags.N = (regs[r] < 0);
    }

    void reset() {
        std::fill(regs.begin(), regs.end(), 0);
        flags = Flags();
        SP = ram.size();
    }
};


#endif