#ifndef ERGON_STEP_HANDLER_H
#define ERGON_STEP_HANDLER_H

#include "computer/core.h"


inline void step_instr(SimpleCore& c, const DecodedInstr& prog) {
    #if !defined(__GNUC__) && !defined(__clang__)
        #error "Computed goto requires GCC or Clang therefore you cannot use AUTO execution mode"
    #endif

        static void* dispatch_table[256] = {
            &&OP_ADD, &&OP_SUB, &&OP_MUL, &&OP_DIV, &&OP_MOD,
            &&OP_ADDI, &&OP_SUBI, &&OP_MULI, &&OP_DIVI, &&OP_MODI,
            &&OP_AND, &&OP_OR, &&OP_XOR, &&OP_ANDI, &&OP_ORI, &&OP_XORI,
            &&OP_SHL, &&OP_SHR, &&OP_SAR, &&OP_ROL, &&OP_ROR,
            &&OP_SHLI, &&OP_SHRI, &&OP_SARI, &&OP_ROLI, &&OP_RORI,
            &&OP_CMP, &&OP_CMPU, &&OP_TEST,
            &&OP_CMPI, &&OP_CMPUI, &&OP_TESTI,
            &&OP_INC, &&OP_DEC, &&OP_NOT, &&OP_ABS, &&OP_NEG,
            &&OP_MIN, &&OP_MAX, &&OP_MINI, &&OP_MAXI,
            /* FPU gap */
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            &&OP_MOV_IMM, &&OP_MOV_REG,
            &&OP_LDB_ABS, &&OP_LDH_ABS, &&OP_LDW_ABS,
            &&OP_STB_ABS, &&OP_STH_ABS, &&OP_STW_ABS,
            &&OP_LDB_BASE, &&OP_LDH_BASE, &&OP_LDW_BASE,
            &&OP_STB_BASE, &&OP_STH_BASE, &&OP_STW_BASE,
            &&OP_PUSH, &&OP_POP,
            &&OP_LEA, &&OP_SWAP, &&OP_CLR, &&OP_MEMCPY,
            &&OP_JMP, &&OP_JZ, &&OP_JNZ, &&OP_JG, &&OP_JL,
            &&OP_CALL, &&OP_RET,
            &&OP_HALT
        };

    #define DISPATCH() goto *dispatch_table[instr->opcode]

    #define STEP() c.PC++;

    const DecodedInstr* instr = &prog;
    DISPATCH();

OP_ADD:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] + (uint64_t)c.regs[instr->rs2]);
    STEP();
OP_SUB:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] - (uint64_t)c.regs[instr->rs2]);
    STEP();
OP_MUL:
    c.regs[instr->rd] = (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] * (int64_t)(int32_t)c.regs[instr->rs2]);
    STEP();
OP_DIV:
    if ((int32_t)c.regs[instr->rs2] != 0 && !((int32_t)c.regs[instr->rs1] == INT32_MIN && (int32_t)c.regs[instr->rs2] == -1))
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] / (int32_t)c.regs[instr->rs2]);
    STEP();
OP_MOD:
    if ((int32_t)c.regs[instr->rs2] != 0)
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] % (int32_t)c.regs[instr->rs2]);
    STEP();
OP_ADDI:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] + (uint64_t)instr->rs2);
    STEP();
OP_SUBI:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] - (uint64_t)(int32_t)instr->rs2);
    STEP();
OP_MULI:
    c.regs[instr->rd] = (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] * (int64_t)(int32_t)instr->rs2);
    STEP();
OP_DIVI:
    if ((int32_t)instr->rs2 != 0 && !((int32_t)c.regs[instr->rs1] == INT32_MIN && (int32_t)instr->rs2 == -1))
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] / (int32_t)instr->rs2);
    STEP();
OP_MODI:
    if ((int32_t)instr->rs2 != 0) c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] % (int32_t)instr->rs2);
    STEP();

OP_AND:
    c.regs[instr->rd] = c.regs[instr->rs1] & c.regs[instr->rs2];
    STEP();
OP_OR:
    c.regs[instr->rd] = c.regs[instr->rs1] | c.regs[instr->rs2];
    STEP();
OP_XOR:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ c.regs[instr->rs2];
    STEP();
OP_ANDI:
    c.regs[instr->rd] = c.regs[instr->rs1] & (uint32_t)(int32_t)instr->rs2;
    STEP();
OP_ORI:
    c.regs[instr->rd] = c.regs[instr->rs1] | (uint32_t)(int32_t)instr->rs2;
    STEP();
OP_XORI:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ (uint32_t)(int32_t)instr->rs2;
    STEP();

OP_SHL:
    c.regs[instr->rd] = c.regs[instr->rs1] << (c.regs[instr->rs2] & 31);
    STEP();
OP_SHR:
    c.regs[instr->rd] = c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31);
    STEP();
OP_SAR:
    c.regs[instr->rd] = static_cast<uint32_t>(static_cast<int32_t>(c.regs[instr->rs1]) >> (c.regs[instr->rs2] & 31));
    STEP();
OP_ROL:
    c.regs[instr->rd] = (c.regs[instr->rs1] << (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] >> (32 - (c.regs[instr->rs2] & 31)));
    STEP();
OP_ROR:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] << (32 - (c.regs[instr->rs2] & 31)));
    STEP();
OP_SHLI:
    c.regs[instr->rd] = c.regs[instr->rs1] << ((uint32_t)instr->rs2 & 31);
    STEP();
OP_SHRI:
    c.regs[instr->rd] = c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31);
    STEP();
OP_SARI:
    c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31));
    STEP();
OP_ROLI:
    c.regs[instr->rd] = (c.regs[instr->rs1] << ((uint32_t)instr->rs2 & 31)) | (c.regs[instr->rs1] >> (32 - ((uint32_t)instr->rs2 & 31)));
    STEP();
OP_RORI:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31)) | (c.regs[instr->rs1] << (32 - ((uint32_t)instr->rs2 & 31)));
    STEP();
OP_CMP:
    c.regs[13] = ((int32_t)c.regs[instr->rs1] < (int32_t)c.regs[instr->rs2]) ? 1 : 0;
    STEP();
OP_CMPU:
    c.regs[13] = (c.regs[instr->rs1] < c.regs[instr->rs2]) ? 1 : 0;
    STEP();
OP_CMPI:
    c.regs[13] = ((int32_t)c.regs[instr->rs1] < (int32_t)instr->rs2) ? 1 : 0;
    STEP();
OP_CMPUI:
    c.regs[13] = (c.regs[instr->rs1] < (uint32_t)instr->rs2) ? 1 : 0;
    STEP();
OP_TEST:
    c.regs[13] = ((c.regs[instr->rs1] & c.regs[instr->rs2]) != 0) ? 1 : 0;
    STEP();
OP_TESTI:
    c.regs[13] = ((c.regs[instr->rs1] & (uint32_t)instr->rs2) != 0) ? 1 : 0;
    STEP();

OP_INC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] + 1;
    STEP();
OP_DEC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] - 1;
    STEP();
OP_NOT:
    c.regs[instr->rd] = ~c.regs[instr->rs1];
    STEP();
OP_ABS:
    if ((int32_t)c.regs[instr->rs1] < 0) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs1];
    STEP();
OP_NEG:
    if ((int32_t)c.regs[instr->rs1] != INT32_MIN) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    STEP();
OP_MIN:
    if (c.regs[instr->rs1] < c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    STEP();
OP_MAX:
    if (c.regs[instr->rs1] > c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    STEP();
OP_MINI:
    if (c.regs[instr->rs1] < instr->rs2) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->rs2;
    STEP();
OP_MAXI:
    if (c.regs[instr->rs1] > instr->rs2) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->rs2;
    STEP();

OP_MOV_IMM:
    c.regs[instr->rd] = instr->imm;
    STEP();
OP_MOV_REG:
    c.regs[instr->rd] = c.regs[instr->rs1];
    STEP();
OP_LDB_ABS:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(instr->imm));
    STEP();
OP_LDH_ABS:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(instr->imm));
    STEP();
OP_LDW_ABS:
    c.regs[instr->rd] = c.load32(instr->imm);
    STEP();
OP_STB_ABS:
    c.store8(instr->imm, c.regs[instr->rd] & 0xFF);
    STEP();
OP_STH_ABS:
    c.store16(instr->imm, c.regs[instr->rd] & 0xFFFF);
    STEP();
OP_STW_ABS:
    c.store32(instr->imm, c.regs[instr->rd]);
    STEP();
OP_LDB_BASE:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm)));
    STEP();
OP_LDH_BASE:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm)));
    STEP();
OP_LDW_BASE:
    c.regs[instr->rd] = c.load32(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm));
    STEP();
OP_STB_BASE:
    c.store8(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd] & 0xFF);
    STEP();
OP_STH_BASE:
    c.store16(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd] & 0xFFFF);
    STEP();
OP_STW_BASE:
    c.store32(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd]);
    STEP();

OP_PUSH:
    c.SP -= 4;
    c.store32(c.SP, c.regs[instr->rs1]);
    STEP();
OP_POP:
    c.regs[instr->rd] = c.load32(c.SP);
    c.SP += 4;
    STEP();
OP_LEA:
    c.regs[instr->rd] = c.regs[instr->rs1] + static_cast<int8_t>(instr->imm);
    STEP();
OP_SWAP:
    std::swap(c.regs[instr->rd], c.regs[instr->rs1]);
    STEP();
OP_CLR:
    c.regs[instr->rd] = 0;
    STEP();
OP_MEMCPY:
    {
        int32_t len = static_cast<int8_t>(instr->imm);
        if (len >= 0) {
            for (uint32_t index = 0; index < instr->rs2; ++index)
                if (c.regs[instr->rd] + index < c.ram.size() && c.regs[instr->rs1] + index < c.ram.size())
                    c.ram[c.regs[instr->rd] + index] = c.ram[c.regs[instr->rs1] + index];
        }
    }
    STEP();

OP_JMP:
    c.PC += instr->imm;
    DISPATCH();
OP_JZ:
    if(c.regs[13] == 0)
        c.PC += instr->imm; DISPATCH();
    STEP();
OP_JNZ:
    if(c.regs[13] != 0)
        c.PC += instr->imm; DISPATCH();
    STEP();
OP_JL:
    if((int32_t)c.regs[13] < 0)
        c.PC += instr->imm; DISPATCH();
    STEP();
OP_JG:
    if((int32_t)c.regs[13] > 0)
        c.PC += instr->imm; DISPATCH();
    STEP();
OP_CALL:
    c.SP -= 4;
    c.store32(c.SP, c.PC + 1);
    c.PC += instr->imm;
    DISPATCH();
OP_RET:
    c.PC = c.load32(c.SP);
    c.SP += 4;
    DISPATCH();

OP_HALT:
    return;
}

#endif