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

    //A FAIRE

    //----------------- MEMORY OPERATIONS -----------------
    MOV_IMM , // mov rd, imm
    MOV_REG , // mov rd, rs1
    LDB_ABS , // load byte rd, [imm]
    LDH_ABS , // load half-word rd, [imm]
    LDW_ABS , // load word rd, [imm]
    STB_ABS , // store byte rd, [imm]
    STH_ABS , // store half-word rd, [imm]
    STW_ABS , // store word rd, [imm]
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
    JMP , // jmp {(rd, rs1, rs2) = 24b} (RELATIVE JUMP)
    JZ  , // jz {(rd, rs1, rs2) = 24b} (JMPR if Z flag is true)
    JNZ , // jnz {(rd, rs1, rs2) = 24b} (JMPR if Z flag is false)
    JG  , // jg {(rd, rs1, rs2) = 24b} (JMPR if both Z and N flag are false)
    JL  , // jl {(rd, rs1, rs2) = 24b} (JMPR if N flag is true)
    CALL, // calL {(rd, rs1, rs2) = 24b}  (JMPR and saves the current PC)
    RET , // ret (load previous PC and JMPR there)

    HALT // halt (stops program)
};


template <size_t RAM_SIZE>
struct SimpleCore {
    std::array<uint32_t, 16> regs{};
    uint32_t& PC = regs[16 - 1]; // Program Counter
    uint32_t& SP = regs[16 - 2]; // Stack Pointer
    Flags flags;

    ALU alu;

    inline void R_type_instr(ALUOp op, uint8_t rd, uint8_t rs1, uint8_t rs2) {
        auto r = alu.exec(op, regs[rs1], regs[rs2]);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }
    inline void I_type_instr(ALUOp op, uint8_t rd, uint8_t rs, uint8_t imm) {
        int32_t imm32 = static_cast<int8_t>(imm);
        auto r = alu.exec(op, regs[rs], imm32);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }
    inline void J_type_instr(ALUOp op, uint8_t rd, uint8_t rs) {
        auto r = alu.exec(op, regs[rs], regs[rs]);
        if (r.writeback) regs[rd] = r.value;
        flags = r.flags;
    }

    // <- memory[addr]
    static uint32_t load32(const std::vector<uint8_t>& memory, uint32_t addr) {
        return memory[addr] |
               (memory[addr + 1] << 8) |
               (memory[addr + 2] << 16) |
               (memory[addr + 3] << 24);
    }
    static uint16_t load16(const std::vector<uint8_t>& memory, uint32_t addr) {
        return memory[addr] |
               (memory[addr + 1] << 8);
    }
    static uint8_t load8(const std::vector<uint8_t>& memory, uint32_t addr) {
        return memory[addr];
    }
    // value -> memory
    static void store32(std::vector<uint8_t>& memory, uint32_t addr, uint32_t value) {
        memory[addr] = value & 0xFF;
        memory[addr + 1] = (value >> 8) & 0xFF;
        memory[addr + 2] = (value >> 16) & 0xFF;
        memory[addr + 3] = (value >> 24) & 0xFF;
    }
    static void store16(std::vector<uint8_t>& memory, uint32_t addr, uint16_t value) {
        memory[addr] = value & 0xFF;
        memory[addr + 1] = (value >> 8) & 0xFF;
    }
    static void store8(std::vector<uint8_t>& memory, uint32_t addr, uint8_t value) {
        memory[addr] = value;
    }

    static int32_t sign_extend_24b(uint32_t value) {
        if (value & 0x800000) // bit 23 = sign
            return static_cast<int32_t>(value | 0xFF000000); // extend sign
        else
            return static_cast<int32_t>(value);
    }

    void reset() {
        std::fill(regs.begin(), regs.end(), 0);
        flags = Flags();
    }

    bool step(uint32_t instr, std::vector<uint8_t>& ram) {
        uint8_t opcode = instr & 0xFF; //16 bits instruction index
        uint8_t rd = (instr >> 8) & 0xFF; //index of the "destination" register
        uint8_t rs1 = (instr >> 16) & 0xFF; //index of the first "source" register
        //rs2 = imm because it depends if instr is R or I type (see above)
        uint8_t rs2 = (instr >> 24) & 0xFF; //index of the second "source" register
        uint8_t imm = rs2; //immediate value

        /* ----------------- SCHEMATIC -----------------
         * INSTRUCTION -> { OPCODE , RD, RS1, RS2/IMM }
         *    32b    =        8b  + 8b + 8b +   8b
         *
         * SWITCH:
         *      - ALU OPERATIONS
         *      - FPU OPERATIONS
         *      - MEMORY OPERATIONS
         *
         */

        bool pc_advanced = true;
        switch (opcode) {

        //----------------- ALU OPERATIONS -----------------
        case ADD:
            R_type_instr(ALUOp::ADD, rd, rs1, rs2); break;
        case SUB:
            R_type_instr(ALUOp::SUB, rd, rs1, rs2); break;
        case MUL:
            R_type_instr(ALUOp::MUL, rd, rs1, rs2); break;
        case DIV:
            R_type_instr(ALUOp::DIV, rd, rs1, rs2); break;
        case MOD:
            R_type_instr(ALUOp::MOD, rd, rs1, rs2); break;
        case ADDI:
            I_type_instr(ALUOp::ADD, rd, rs1, imm); break;
        case SUBI:
            I_type_instr(ALUOp::SUB, rd, rs1, imm); break;
        case MULI:
            I_type_instr(ALUOp::MUL, rd, rs1, imm); break;
        case DIVI:
            I_type_instr(ALUOp::DIV, rd, rs1, imm); break;
        case MODI:
            I_type_instr(ALUOp::MOD, rd, rs1, imm); break;

        case AND:
            R_type_instr(ALUOp::AND, rd, rs1, rs2); break;
        case OR:
            R_type_instr(ALUOp::OR, rd, rs1, rs2); break;
        case XOR:
            R_type_instr(ALUOp::XOR, rd, rs1, rs2); break;
        case ANDI:
            I_type_instr(ALUOp::AND, rd, rs1, imm); break;
        case ORI:
            I_type_instr(ALUOp::OR, rd, rs1, imm); break;
        case XORI:
            I_type_instr(ALUOp::XOR, rd, rs1, imm); break;

        case SHL:
            R_type_instr(ALUOp::SHL, rd, rs1, rs2); break;
        case SHR:
            R_type_instr(ALUOp::SHR, rd, rs1, rs2); break;
        case SAR:
            R_type_instr(ALUOp::SAR, rd, rs1, rs2); break;
        case ROL:
            R_type_instr(ALUOp::ROL, rd, rs1, rs2); break;
        case ROR:
            R_type_instr(ALUOp::ROR, rd, rs1, rs2); break;
        case SHLI:
            I_type_instr(ALUOp::SHL, rd, rs1, imm); break;
        case SHRI:
            I_type_instr(ALUOp::SHR, rd, rs1, imm); break;
        case SARI:
            I_type_instr(ALUOp::SAR, rd, rs1, imm); break;
        case ROLI:
            I_type_instr(ALUOp::ROL, rd, rs1, imm); break;
        case RORI:
            I_type_instr(ALUOp::ROR, rd, rs1, imm); break;

        case CMP:
            R_type_instr(ALUOp::CMP, rd, rs1, rs2); break;
        case CMPU:
            R_type_instr(ALUOp::CMPU, rd, rs1, rs2); break;
        case TEST:
            R_type_instr(ALUOp::TEST, rd, rs1, rs2); break;
        case CMPI:
            I_type_instr(ALUOp::CMP, rd, rs1, imm); break;
        case CMPUI:
            I_type_instr(ALUOp::CMPU, rd, rs1, imm); break;
        case TESTI:
            I_type_instr(ALUOp::TEST, rd, rs1, imm); break;

        case INC:
            J_type_instr(ALUOp::INC, rd, rd); break;
        case DEC:
            J_type_instr(ALUOp::DEC, rd, rd); break;
        case NOT:
            J_type_instr(ALUOp::NOT, rd, rs1); break;
        case ABS:
            J_type_instr(ALUOp::ABS, rd, rs1); break;
        case NEG:
            J_type_instr(ALUOp::NEG, rd, rs1); break;
        case MIN:
            R_type_instr(ALUOp::MIN, rd, rs1, rs2); break;
        case MAX:
            R_type_instr(ALUOp::MAX, rd, rs1, rs2); break;
        case MINI:
            I_type_instr(ALUOp::MIN, rd, rs1, imm); break;
        case MAXI:
            I_type_instr(ALUOp::MAX, rd, rs1, imm); break;

        //----------------- FPU OPERATIONS -----------------
        //A FAIRE

        //----------------- MEMORY OPERATIONS -----------------
        case MOV_IMM:
            regs[rd] = imm;
            flags.Z = (imm == 0);
            flags.N = (imm >> 7) & 1;
            break;
        case MOV_REG:
            regs[rd] = regs[rs1];
            break;
        case LDB_ABS:
            regs[rd] = static_cast<int8_t>(load8(ram, imm));
            break;

        case LDH_ABS:
            regs[rd] = static_cast<int16_t>(load16(ram, imm));
            break;

        case LDW_ABS:
            regs[rd] = load32(ram, imm);
            break;
        case STB_ABS:
            store8(ram, imm, regs[rd] & 0xFF);
            break;
        case STH_ABS:
            store16(ram, imm, regs[rd] & 0xFFFF);
            break;
        case STW_ABS:
            store32(ram, imm, regs[rd]);
            break;
        case LDB_BASE:
            regs[rd] = static_cast<int8_t>(load8(ram, regs[rs1] + static_cast<int8_t>(imm)));
            break;
        case LDH_BASE:
            regs[rd] = static_cast<int16_t>(load16(ram, regs[rs1] + static_cast<int8_t>(imm)));
            break;
        case LDW_BASE:
            regs[rd] = load32(ram, regs[rs1] + static_cast<int8_t>(imm));
            break;

        case STB_BASE:
            store8(ram, regs[rs1] + static_cast<int8_t>(imm), regs[rd] & 0xFF);
            break;

        case STH_BASE:
            store16(ram, regs[rs1] + static_cast<int8_t>(imm), regs[rd] & 0xFFFF);
            break;

        case STW_BASE:
            store32(ram, regs[rs1] + static_cast<int8_t>(imm), regs[rd]);
            break;
        case PUSH:
            SP -= 1;
            store32(ram, SP, regs[rs1]);
            break;
        case POP:
            regs[rd] = load32(ram, SP);
            SP += 1;
            break;
        // Load Effective Address
        case LEA:
            regs[rd] = regs[rs1] + static_cast<int8_t>(imm);
            break;
        // Swap registers
        case SWAP:
            std::swap(regs[rd], regs[rs1]);
            break;
        // Clear register
        case CLR:
            regs[rd] = 0;
            flags.Z = true;
            flags.N = false;
            break;
        // Memory copy (rd = destination, rs1 = source, imm = length)
        case MEMCPY:
            for (uint32_t i = 0; i < imm; ++i)
                ram[regs[rd] + i] = ram[regs[rs1] + i];
            break;

        //----------------- PROGRAM OPERATIONS -----------------
        case JMP:
            PC += sign_extend_24b(instr >> 8); //relative jump
            pc_advanced = false;
            break;
        case JZ:
            if (flags.Z) {
                PC += sign_extend_24b(instr >> 8);
                pc_advanced = false;
            }
            break;
        case JNZ:
            if (!flags.Z) {
                PC += sign_extend_24b(instr >> 8);
                pc_advanced = false;
            }
            break;
        case JG:
            if (!flags.Z && !flags.N) {
                PC += sign_extend_24b(instr >> 8);
                pc_advanced = false;
            }
            break;
        case JL:
            if (flags.N) {
                PC += sign_extend_24b(instr >> 8);
                pc_advanced = false;
            }
            break;
        case CALL:
            // pushes PC + 1 on pile
            SP -= 1;
            store32(ram, SP, PC + 1);
            PC += sign_extend_24b(instr >> 8);
            pc_advanced = false;
            break;
        case RET:
            {
                uint32_t return_addr = load32(ram, SP);
                SP += 1;
                PC = return_addr;
                pc_advanced = false;
                break;
            }

        case HALT:
            PC = 0;
            return false;
        default:
            break;
        }
        if (pc_advanced) PC++;

        return true;
    }
};


#endif