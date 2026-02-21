#ifndef ERGON_RUN_HANDLER_H
#define ERGON_RUN_HANDLER_H

#include "computer/core.h"

constexpr uint32_t MASK32 = 31;

inline void run(SimpleCore& c, const std::vector<DecodedInstr>& prog) {
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

    //magie noire >w<
    //#define FETCH() instr = &prog[c.PC]
    #define FETCH() instr = &prog[c.PC];

    #define DISPATCH() goto *dispatch_table[instr->opcode]

    //#define NEXT() do { c.PC++; FETCH(); DISPATCH(); } while (0)
    #define NEXT() \
    if (c.PC + 1 >= prog.size()) return; \
    c.PC++; FETCH(); DISPATCH();

    const DecodedInstr* instr;
    FETCH();
    DISPATCH();

OP_ADD:
    c.regs[instr->rd] =
        (uint32_t)((uint64_t)c.regs[instr->rs1] + (uint64_t)c.regs[instr->rs2]);
    c.flags.C =
        ((uint64_t)c.regs[instr->rs1] + (uint64_t)c.regs[instr->rs2]) > 0xFFFFFFFF;
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] + (int64_t)(int32_t)c.regs[instr->rs2]) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] + (int64_t)(int32_t)c.regs[instr->rs2]) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SUB:
    c.regs[instr->rd] =
        (uint32_t)((uint64_t)c.regs[instr->rs1] - (uint64_t)c.regs[instr->rs2]);
    c.flags.C = c.regs[instr->rs1] >= c.regs[instr->rs2];
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] - (int64_t)(int32_t)c.regs[instr->rs2]) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] - (int64_t)(int32_t)c.regs[instr->rs2]) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MUL:
    c.regs[instr->rd] =
        (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] *
                   (int64_t)(int32_t)c.regs[instr->rs2]);
    c.flags.C = false;
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] *
         (int64_t)(int32_t)c.regs[instr->rs2]) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] *
         (int64_t)(int32_t)c.regs[instr->rs2]) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_DIV:
    if ((int32_t)c.regs[instr->rs2] != 0 &&
        !((int32_t)c.regs[instr->rs1] == INT32_MIN &&
          (int32_t)c.regs[instr->rs2] == -1))
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] / (int32_t)c.regs[instr->rs2]);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MOD:
    if ((int32_t)c.regs[instr->rs2] != 0)
        c.regs[instr->rd] =
            (uint32_t)((int32_t)c.regs[instr->rs1] % (int32_t)c.regs[instr->rs2]);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
    OP_ADDI:
    c.regs[instr->rd] =
        (uint32_t)((uint64_t)c.regs[instr->rs1] +
                   (uint64_t)(int32_t)instr->rs2);
    c.flags.C =
        ((uint64_t)c.regs[instr->rs1] +
         (uint64_t)(int32_t)instr->rs2) > 0xFFFFFFFF;
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] +
         (int64_t)(int32_t)instr->rs2) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] +
         (int64_t)(int32_t)instr->rs2) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SUBI:
    c.regs[instr->rd] =
        (uint32_t)((uint64_t)c.regs[instr->rs1] -
                   (uint64_t)(int32_t)instr->rs2);
    c.flags.C =
        c.regs[instr->rs1] >= (uint32_t)(int32_t)instr->rs2;
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] -
         (int64_t)(int32_t)instr->rs2) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] -
         (int64_t)(int32_t)instr->rs2) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MULI:
    c.regs[instr->rd] =
        (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] *
                   (int64_t)(int32_t)instr->rs2);
    c.flags.C = false;
    c.flags.V =
        ((int64_t)(int32_t)c.regs[instr->rs1] *
         (int64_t)(int32_t)instr->rs2) > INT32_MAX ||
        ((int64_t)(int32_t)c.regs[instr->rs1] *
         (int64_t)(int32_t)instr->rs2) < INT32_MIN;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_DIVI:
    if ((int32_t)instr->rs2 != 0 &&
        !((int32_t)c.regs[instr->rs1] == INT32_MIN &&
          (int32_t)instr->rs2 == -1))
        c.regs[instr->rd] =
            (uint32_t)((int32_t)c.regs[instr->rs1] /
                       (int32_t)instr->rs2);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MODI:
    if ((int32_t)instr->rs2 != 0)
        c.regs[instr->rd] =
            (uint32_t)((int32_t)c.regs[instr->rs1] %
                       (int32_t)instr->rs2);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();

OP_AND:
    c.regs[instr->rd] = c.regs[instr->rs1] & c.regs[instr->rs2];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_OR:
    c.regs[instr->rd] = c.regs[instr->rs1] | c.regs[instr->rs2];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_XOR:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ c.regs[instr->rs2];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ANDI:
    c.regs[instr->rd] = c.regs[instr->rs1] & (uint32_t)(int32_t)instr->rs2;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ORI:
    c.regs[instr->rd] = c.regs[instr->rs1] | (uint32_t)(int32_t)instr->rs2;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_XORI:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ (uint32_t)(int32_t)instr->rs2;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();

OP_SHL:
    c.regs[instr->rd] = c.regs[instr->rs1] << (c.regs[instr->rs2] & 31);
    c.flags.C = (c.regs[instr->rs1] >> (32 - (c.regs[instr->rs2] & 31))) & 1;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SHR:
    c.regs[instr->rd] = c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31);
    c.flags.C = (c.regs[instr->rs1] >> ((c.regs[instr->rs2] & 31) - 1)) & 1;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SAR:
    c.regs[instr->rd] = static_cast<uint32_t>(static_cast<int32_t>(c.regs[instr->rs1]) >> (c.regs[instr->rs2] & 31));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ROL:
    c.regs[instr->rd] = (c.regs[instr->rs1] << (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] >> (32 - (c.regs[instr->rs2] & 31)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ROR:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] << (32 - (c.regs[instr->rs2] & 31)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SHLI:
    c.regs[instr->rd] = c.regs[instr->rs1] << ((uint32_t)instr->rs2 & 31);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SHRI:
    c.regs[instr->rd] = c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SARI:
    c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ROLI:
    c.regs[instr->rd] = (c.regs[instr->rs1] << ((uint32_t)instr->rs2 & 31)) | (c.regs[instr->rs1] >> (32 - ((uint32_t)instr->rs2 & 31)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_RORI:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> ((uint32_t)instr->rs2 & 31)) | (c.regs[instr->rs1] << (32 - ((uint32_t)instr->rs2 & 31)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();

OP_CMP:
    c.flags.Z = ((int32_t)c.regs[instr->rs1] - (int32_t)c.regs[instr->rs2]) == 0;
    c.flags.N = ((int32_t)c.regs[instr->rs1] - (int32_t)c.regs[instr->rs2]) < 0;
    c.flags.C = c.regs[instr->rs1] >= c.regs[instr->rs2];
    c.flags.V = (((int32_t)c.regs[instr->rs1] ^ (int32_t)c.regs[instr->rs2]) & ((int32_t)c.regs[instr->rs1] ^ ((int32_t)c.regs[instr->rs1] - (int32_t)c.regs[instr->rs2]))) < 0;
    NEXT();
OP_CMPU:
    c.flags.Z = c.regs[instr->rs1] == c.regs[instr->rs2];
    c.flags.N = c.regs[instr->rs1] < c.regs[instr->rs2];
    c.flags.C = c.regs[instr->rs1] >= c.regs[instr->rs2];
    c.flags.V = ((c.regs[instr->rs1] ^ c.regs[instr->rs2]) & (c.regs[instr->rs1] ^ (c.regs[instr->rs1] - c.regs[instr->rs2]))) < 0;
    NEXT();
OP_TEST:
    c.flags.Z = ((c.regs[instr->rs1] & c.regs[instr->rs2]) == 0);
    c.flags.N = ((c.regs[instr->rs1] & c.regs[instr->rs2]) >> 31) & 1;
    c.flags.C = false;
    c.flags.V = false;
    NEXT();
OP_CMPI:
    c.flags.Z = ((int32_t)c.regs[instr->rs1] - (int32_t)instr->rs2) == 0;
    c.flags.N = ((int32_t)c.regs[instr->rs1] - (int32_t)instr->rs2) < 0;
    c.flags.C = c.regs[instr->rs1] >= (uint32_t)(int32_t)instr->rs2;
    c.flags.V = (((int32_t)c.regs[instr->rs1] ^ (int32_t)instr->rs2) & ((int32_t)c.regs[instr->rs1] ^ ((int32_t)c.regs[instr->rs1] - (int32_t)instr->rs2))) < 0;
    NEXT();
OP_CMPUI:
    c.flags.Z = c.regs[instr->rs1] == (uint32_t)(int32_t)instr->rs2;
    c.flags.N = c.regs[instr->rs1] < (uint32_t)(int32_t)instr->rs2;
    c.flags.C = c.regs[instr->rs1] >= (uint32_t)(int32_t)instr->rs2;
    c.flags.V = ((c.regs[instr->rs1] ^ (uint32_t)(int32_t)instr->rs2) & (c.regs[instr->rs1] ^ (c.regs[instr->rs1] - (uint32_t)(int32_t)instr->rs2))) < 0;
    NEXT();
OP_TESTI:
    c.flags.Z = ((c.regs[instr->rs1] & (uint32_t)(int32_t)instr->rs2) == 0);
    c.flags.N = ((c.regs[instr->rs1] & (uint32_t)(int32_t)instr->rs2) >> 31) & 1;
    c.flags.C = false;
    c.flags.V = false;
    NEXT();

OP_INC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] + 1;
    c.flags.C = false;
    c.flags.V = (int64_t)(int32_t)c.regs[instr->rd] + 1 > INT32_MAX;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_DEC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] - 1;
    c.flags.C = false;
    c.flags.V = (int64_t)(int32_t)c.regs[instr->rd] - 1 > INT32_MAX;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_NOT:
    c.regs[instr->rd] = ~c.regs[instr->rs1];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_ABS:
    if ((int32_t)c.regs[instr->rs1] < 0) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs1];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_NEG:
    if ((int32_t)c.regs[instr->rs1] != INT32_MIN) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MIN:
    if (c.regs[instr->rs1] < c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MAX:
    if (c.regs[instr->rs1] > c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MINI:
    if (c.regs[instr->rs1] < instr->rs2) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->rs2;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MAXI:
    if (c.regs[instr->rs1] > instr->rs2) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->rs2;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();

OP_MOV_IMM:
    c.regs[instr->rd] = instr->imm;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_MOV_REG:
    c.regs[instr->rd] = c.regs[instr->rs1];
    NEXT();
OP_LDB_ABS:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(instr->imm));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_LDH_ABS:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(instr->imm));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_LDW_ABS:
    c.regs[instr->rd] = c.load32(instr->imm);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_STB_ABS:
    c.store8(instr->imm, c.regs[instr->rd] & 0xFF);
    NEXT();
OP_STH_ABS:
    c.store16(instr->imm, c.regs[instr->rd] & 0xFFFF);
    NEXT();
OP_STW_ABS:
    c.store32(instr->imm, c.regs[instr->rd]);
    NEXT();
OP_LDB_BASE:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_LDH_BASE:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm)));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_LDW_BASE:
    c.regs[instr->rd] = c.load32(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm));
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_STB_BASE:
    c.store8(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd] & 0xFF);
    NEXT();
OP_STH_BASE:
    c.store16(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd] & 0xFFFF);
    NEXT();
OP_STW_BASE:
    c.store32(c.regs[instr->rs1] + static_cast<int8_t>(instr->imm), c.regs[instr->rd]);
    NEXT();

OP_PUSH:
    c.SP -= 4;
    c.store32(c.SP, c.regs[instr->rs1]);
    NEXT();
OP_POP:
    c.regs[instr->rd] = c.load32(c.SP);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    c.SP += 4;
    NEXT();
OP_LEA:
    c.regs[instr->rd] = c.regs[instr->rs1] + static_cast<int8_t>(instr->imm);
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = (c.regs[instr->rd] == 0);
    c.flags.N = (c.regs[instr->rd] >> 31) & 1;
    NEXT();
OP_SWAP:
    std::swap(c.regs[instr->rd], c.regs[instr->rs1]);
    NEXT();
OP_CLR:
    c.regs[instr->rd] = 0;
    c.flags.C = false;
    c.flags.V = false;
    c.flags.Z = true;
    c.flags.N = false;
    NEXT();
OP_MEMCPY:
    {
        int32_t len = static_cast<int8_t>(instr->imm);
        if (len >= 0) {
            for (uint32_t index = 0; index < instr->rs2; ++index)
                if (c.regs[instr->rd] + index < c.ram.size() && c.regs[instr->rs1] + index < c.ram.size())
                    c.ram[c.regs[instr->rd] + index] = c.ram[c.regs[instr->rs1] + index];
        }
    }
    NEXT();

OP_JMP:
    c.PC += instr->imm;
    FETCH();
    DISPATCH();
OP_JZ:
    if (c.flags.Z) {
        c.PC += instr->imm;
        FETCH();
        DISPATCH();
    }
    NEXT();
OP_JNZ:
    if (!c.flags.Z) {
        c.PC += instr->imm;
        FETCH();
        DISPATCH();
    }
    NEXT();
OP_JG:
    if (!c.flags.Z && !c.flags.N) {
        c.PC += instr->imm;
        FETCH();
        DISPATCH();
    }
    NEXT();
OP_JL:
    if (c.flags.N) {
        c.PC += instr->imm;
        FETCH();
        DISPATCH();
    }
    NEXT();
OP_CALL:
    c.SP -= 4;
    c.store32(c.SP, c.PC + 1);
    c.PC += instr->imm;
    FETCH();
    DISPATCH();
OP_RET:
    c.PC = c.load32(c.SP);
    c.SP += 4;
    FETCH();
    DISPATCH();

OP_HALT:
    //c.PC = 0xFFFFFFFF;
    return;
}

#endif