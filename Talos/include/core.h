#ifndef ERGON_CORE_H
#define ERGON_CORE_H

#include "alu.h"

#include <cstdint>
#include <array>
#include <vector>


enum OPCODE : uint8_t {
    //----------------- ALU OPERATIONS -----------------
    ADD     = 0x00, // R-type: ADD rd, rs1, rs2
    SUB     = 0x01, // R-type: SUB rd, rs1, rs2
    MUL     = 0x02, // R-type: MUL rd, rs1, rs2
    DIV     = 0x03, // R-type: DIV rd, rs1, rs2
    MOD     = 0x04, // R-type: MOD rd, rs1, rs2
    ADDI    = 0x05, // I-type: ADDI rd, rs, imm
    SUBI    = 0x06, // I-type: SUBI rd, rs, imm
    MULI    = 0x07, // I-type: MULI rd, rs, imm
    DIVI    = 0x08, // I-type: DIVI rd, rs, imm
    MODI    = 0x09, // I-type: MODI rd, rs, imm

    AND     = 0x0A, // R-type: AND rd, rs1, rs2
    OR      = 0x0B, // R-type: OR rd, rs1, rs2
    XOR     = 0x0C, // R-type: XOR rd, rs1, rs2
    ANDI    = 0x0D, // I-type: ANDI rd, rs, imm
    ORI     = 0x0E, // I-type: ORI rd, rs, imm
    XORI    = 0x0F, // I-type: XORI rd, rs, imm

    SHL     = 0x10, // R-type: SHL rd, rs, rt
    SHR     = 0x11, // R-type: SHR rd, rs, rt
    SAR     = 0x12, // R-type: SAR rd, rs, rt
    ROL     = 0x13, // R-type: ROL rd, rs, rt
    ROR     = 0x14, // R-type: ROR rd, rs, rt
    SHLI    = 0x15, // I-type: SHLI rd, rs, imm
    SHRI    = 0x16, // I-type: SHRI rd, rs, imm
    SARI    = 0x17, // I-type: SARI rd, rs, imm
    ROLI    = 0x18, // I-type: ROLI rd, rs, imm
    RORI    = 0x19, // I-type: RORI rd, rs, imm

    CMP     = 0x1A, // R-type: CMP rs1, rs2
    TEST    = 0x1B, // R-type: TEST rs1, rs2
    CMPI    = 0x1C, // I-type: CMPI rs, imm
    TESTI   = 0x1D, // I-type: TESTI rs, imm

    INC     = 0x1E, // J-type: INC rd
    DEC     = 0x1F, // J-type: DEC rd
    NOT     = 0x20, // J-type: NOT rd, rs
    ABS     = 0x21, // J-type: ABS rd, rs
    NEG     = 0x22, // J-type: NEG rd, rs
    MIN     = 0x23, // R-type: MIN rd, rs1, rs2
    MAX     = 0x24, // R-type: MAX rd, rs1, rs2
    MINI    = 0x25, // I-type: MINI rd, rs1, imm
    MAXI    = 0x26, // I-type: MAXI rd, rs1, imm

    //----------------- FPU OPERATIONS -----------------

    //A FAIRE

    //----------------- MEMORY OPERATIONS -----------------
    MOV_IMM = 0x27, // MOV rd, imm
    MOV_REG = 0x28, // MOV rd, rs1
    LOAD    = 0x29, // LOAD rd, [imm]
    STORE   = 0x2A, // STORE rs1, [imm]
    LOAD_BASE = 0x2B, // LOAD rd, [rs1 + imm]
    LDB     = 0x2C, // LOAD byte rd, [rs1 + imm] 8b sign-extended
    LDH     = 0x2D, // LOAD halfword rd, [rs1 + imm] 16b sign-extended
    LDW     = 0x2E, // LOAD word rd, [rs1 + imm] 32b
    STORE_BASE = 0x2F, // STORE rs1, [rd + imm]
    STB     = 0x30, // STORE byte rs1, [rd + imm] 8b
    STH     = 0x31, // STORE halfword rs1, [rd + imm] 16b
    STW     = 0x32, // STORE word rs1, [rd + imm] 32b
    PUSH    = 0x33, // PUSH rs1
    POP     = 0x34, // POP rd
    LEA     = 0x35, // LEA rd, rs1, imm
    SWAP    = 0x36, // SWAP rd, rs1
    CLR     = 0x37, // CLR rd
    MEMCPY  = 0x38, // MEMCPY rd, rs1, imm (length = imm)

    //----------------- PROGRAM OPERATIONS -----------------
    JMP     = 0x39, // JMP {(rd, rs1, rs2) = 24b} (RELATIVE JUMP)
    JZ      = 0x3A, // JZ {(rd, rs1, rs2) = 24b} (JMPR if Z flag is true)
    JNZ     = 0x3B, // JNZ {(rd, rs1, rs2) = 24b} (JMPR if Z flag is false)
    JG      = 0x3C, // JG {(rd, rs1, rs2) = 24b} (JMPR if both Z and N flag are false)
    JL      = 0x3D, // JL {(rd, rs1, rs2) = 24b} (JMPR if N flag is true)
    CALL    = 0x3E, // CALL {(rd, rs1, rs2) = 24b}  (JMPR and saves the current PC)
    RET     = 0x3F, // RET (load previous PC and JMPR there)

    HALT    = 0xFF // HALT (stops program)
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

    bool step(uint32_t instr, std::array<uint8_t, RAM_SIZE>& ram) {
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
        case TEST:
            R_type_instr(ALUOp::TEST, rd, rs1, rs2); break;
        case CMPI:
            I_type_instr(ALUOp::CMP, rd, rs1, imm); break;
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
        case LOAD:
            regs[rd] = ram[imm] |
                (ram[imm + 1] << 8) |
                (ram[imm + 2] << 16) |
                (ram[imm + 3] << 24);
            break;
        case STORE:
            ram[imm] = regs[rs1] & 0xFF;
            ram[imm + 1] = (regs[rs1] >> 8) & 0xFF;
            ram[imm + 2] = (regs[rs1] >> 16) & 0xFF;
            ram[imm + 3] = (regs[rs1] >> 24) & 0xFF;
            break;
        case LOAD_BASE:
            regs[rd] = load32(ram, regs[rs1] + static_cast<int8_t>(imm));
            break;
        case LDB:
            regs[rd] = static_cast<int8_t>(load8(ram, regs[rs1] + static_cast<int8_t>(imm))); // sign-extend
            break;
        case LDH:
            regs[rd] = static_cast<int16_t>(load16(ram, regs[rs1] + static_cast<int8_t>(imm))); // sign-extend
            break;
        case LDW:
            regs[rd] = load32(ram, regs[rs1] + static_cast<int8_t>(imm));
            break;
        case STORE_BASE:
            store32(ram, regs[rd] + static_cast<int8_t>(imm), regs[rs1]);
            break;
        case STB:
            store8(ram, regs[rd] + static_cast<int8_t>(imm), regs[rs1] & 0xFF); //convert regs[rs1] to 8b
            break;
        case STH:
            store16(ram, regs[rd] + static_cast<int8_t>(imm), regs[rs1] & 0xFFFF); //convert regs[rs1] to 16b
            break;
        case STW:
            store32(ram, regs[rd] + static_cast<int8_t>(imm), regs[rs1]);
            break;
        case PUSH:
            SP -= 4;
            store32(ram, SP, regs[rs1]);
            break;
        case POP:
            regs[rd] = load32(ram, SP);
            SP += 4;
            break;
        case LEA:
            regs[rd] = regs[rs1] + static_cast<int8_t>(imm);
            break;
        case SWAP:
            std::swap(regs[rd], regs[rs1]);
            break;
        case CLR:
            regs[rd] = 0;
            flags.Z = true;
            flags.N = false;
            break;
        case MEMCPY:
            for (uint32_t i = 0; i < imm; ++i)
                ram[regs[rd] + i] = ram[regs[rs1] + i];
            break;

        //----------------- PROGRAM OPERATIONS -----------------
        case JMP:
            PC += sign_extend_24b(instr & 0xFFFFFF); //relative jump
            pc_advanced = false;
            break;
        case JZ:
            if (flags.Z) {
                PC += sign_extend_24b(instr & 0xFFFFFF);
                pc_advanced = false;
            }
            break;
        case JNZ:
            if (!flags.Z) {
                PC += sign_extend_24b(instr & 0xFFFFFF);
                pc_advanced = false;
            }
            break;
        case JG:
            if (!flags.Z && !flags.N) {
                PC += sign_extend_24b(instr & 0xFFFFFF);
                pc_advanced = false;
            }
            break;
        case JL:
            if (flags.N) {
                PC += sign_extend_24b(instr & 0xFFFFFF);
                pc_advanced = false;
            }
            break;
        case CALL:
            // pushes PC + 4 on pile
            SP -= 4;
            store32(ram, SP, PC + 4);
            PC += sign_extend_24b(instr & 0xFFFFFF);
            pc_advanced = false;
            break;
        case RET:
            {
                uint32_t return_addr = load32(ram, SP);
                SP += 4;
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
        if (pc_advanced) PC += 4;

        return true;
    }
};


#endif