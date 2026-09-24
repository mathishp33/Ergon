#ifndef ERGON_RUN_HANDLER_H
#define ERGON_RUN_HANDLER_H

#include <bit>
#include <functional>

#include "hardware/core.h"


enum class RunResult {
    HALTED,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    SYSCALL_STOP,
    ERROR,
    PC_OVERFLOW,
};

inline RunResult run(SimpleCore& c) {
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

            &&OP_FADD, &&OP_FSUB, &&OP_FMUL, &&OP_FDIV, &&OP_FMA,
            &&OP_FSQRT, &&OP_FABS, &&OP_FNEG, &&OP_FCMP, &&OP_ITOF, &&OP_FTOI,
            &&OP_FMOV, &&OP_MOVF, &&OP_FLDW_ABS, &&OP_FSDW_ABS,
            &&OP_FLDW_BASE, &&OP_FSDW_BASE, &&OP_FLDW_REG, &&OP_FSDW_REG,

            &&OP_MOV_IMM, &&OP_MOV_REG,
            &&OP_LDB_ABS, &&OP_LDH_ABS, &&OP_LDW_ABS,
            &&OP_STB_ABS, &&OP_STH_ABS, &&OP_STW_ABS,
            &&OP_LDB_BASE, &&OP_LDH_BASE, &&OP_LDW_BASE,
            &&OP_LDB_REG, &&OP_LDH_REG, &&OP_LDW_REG,
            &&OP_STB_BASE, &&OP_STH_BASE, &&OP_STW_BASE,
            &&OP_STB_REG, &&OP_STH_REG, &&OP_STW_REG,

            &&OP_PUSH, &&OP_POP,

            &&OP_LEA, &&OP_LEAB, &&OP_SWAP, &&OP_CLR, &&OP_MEMCPY,

            &&OP_JMP, &&OP_JZ, &&OP_JNZ, &&OP_JG, &&OP_JL,

            &&OP_CALL, &&OP_RET,
            &&OP_SYSCALL, &&OP_HALT,

            &&OP_SETTV, &&OP_SYSRET
        };

    if (c.bus.ram_size() == 0) return RunResult::ERROR;
    //magie noire >w<
    // "instr" reste un pointeur qui pointe vers
    // "instr_storage", qui est ré  écrasée à chaque fetch depuis le BUS.
    #define FETCH() instr_storage = c.bus.fetch_instr(c.PC);

    #define DISPATCH() goto *dispatch_table[instr->opcode]

    #define CHECK_PC() \
    if (c.pending_fault) { \
    c.pending_fault = false; \
    const bool was_user = (c.mode == PrivMode::USER); \
    c.mode = PrivMode::KERNEL; /* push require kernel */ \
    if (c.SP < 4) return RunResult::STACK_OVERFLOW; \
    c.SP -= 4; \
    c.store32(c.SP, c.PC | (was_user ? 1u : 0u)); \
    c.regs[11] = c.fault_cause; \
    c.PC = c.trap_vector; \
    } \
    if (!c.bus.pc_in_bounds(c.PC)) return RunResult::PC_OVERFLOW;

    #define NEXT() \
    c.PC += INSTR_SIZE; \
    CHECK_PC(); \
    FETCH(); DISPATCH();

    DecodedInstr instr_storage;
    const DecodedInstr* instr = &instr_storage;

    CHECK_PC();
    FETCH();
    DISPATCH();

OP_ADD:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] + (uint64_t)c.regs[instr->rs2]);
    NEXT();
OP_SUB:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] - (uint64_t)c.regs[instr->rs2]);
    NEXT();
OP_MUL:
    c.regs[instr->rd] = (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] * (int64_t)(int32_t)c.regs[instr->rs2]);
    NEXT();
OP_DIV:
    if ((int32_t)c.regs[instr->rs2] != 0 && !((int32_t)c.regs[instr->rs1] == INT32_MIN && (int32_t)c.regs[instr->rs2] == -1))
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] / (int32_t)c.regs[instr->rs2]);
    NEXT();
OP_MOD:
    if ((int32_t)c.regs[instr->rs2] != 0)
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] % (int32_t)c.regs[instr->rs2]);
    NEXT();
OP_ADDI:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] + (uint64_t)instr->imm);
    NEXT();
OP_SUBI:
    c.regs[instr->rd] = (uint32_t)((uint64_t)c.regs[instr->rs1] - (uint64_t)instr->imm);
    NEXT();
OP_MULI:
    c.regs[instr->rd] = (uint32_t)((int64_t)(int32_t)c.regs[instr->rs1] * (int64_t)instr->imm);
    NEXT();
OP_DIVI:
    if (instr->imm != 0 && !((int32_t)c.regs[instr->rs1] == INT32_MIN && instr->imm == -1))
        c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] / instr->imm);
    NEXT();
OP_MODI:
    if (instr->imm != 0) c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] % instr->imm);
    NEXT();

OP_AND:
    c.regs[instr->rd] = c.regs[instr->rs1] & c.regs[instr->rs2];
    NEXT();
OP_OR:
    c.regs[instr->rd] = c.regs[instr->rs1] | c.regs[instr->rs2];
    NEXT();
OP_XOR:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ c.regs[instr->rs2];
    NEXT();
OP_ANDI:
    c.regs[instr->rd] = c.regs[instr->rs1] & (uint32_t)instr->imm;
    NEXT();
OP_ORI:
    c.regs[instr->rd] = c.regs[instr->rs1] | (uint32_t)instr->imm;
    NEXT();
OP_XORI:
    c.regs[instr->rd] = c.regs[instr->rs1] ^ (uint32_t)instr->imm;
    NEXT();

OP_SHL:
    c.regs[instr->rd] = c.regs[instr->rs1] << (c.regs[instr->rs2] & 31);
    NEXT();
OP_SHR:
    c.regs[instr->rd] = c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31);
    NEXT();
OP_SAR:
    c.regs[instr->rd] = static_cast<uint32_t>(static_cast<int32_t>(c.regs[instr->rs1]) >> (c.regs[instr->rs2] & 31));
    NEXT();
OP_ROL:
    c.regs[instr->rd] = (c.regs[instr->rs1] << (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] >> (32 - (c.regs[instr->rs2] & 31)));
    NEXT();
OP_ROR:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> (c.regs[instr->rs2] & 31)) | (c.regs[instr->rs1] << (32 - (c.regs[instr->rs2] & 31)));
    NEXT();
OP_SHLI:
    c.regs[instr->rd] = c.regs[instr->rs1] << ((uint32_t)instr->imm & 31);
    NEXT();
OP_SHRI:
    c.regs[instr->rd] = c.regs[instr->rs1] >> ((uint32_t)instr->imm & 31);
    NEXT();
OP_SARI:
    c.regs[instr->rd] = (uint32_t)((int32_t)c.regs[instr->rs1] >> ((uint32_t)instr->imm & 31));
    NEXT();
OP_ROLI:
    c.regs[instr->rd] = (c.regs[instr->rs1] << ((uint32_t)instr->imm & 31)) | (c.regs[instr->rs1] >> (32 - ((uint32_t)instr->imm & 31)));
    NEXT();
OP_RORI:
    c.regs[instr->rd] = (c.regs[instr->rs1] >> ((uint32_t)instr->imm & 31)) | (c.regs[instr->rs1] << (32 - ((uint32_t)instr->imm & 31)));
    NEXT();
OP_CMP:
    c.regs[12] = ((int32_t)c.regs[instr->rs1] < (int32_t)c.regs[instr->rs2]) ? -1 : (((int32_t)c.regs[instr->rs1] > (int32_t)c.regs[instr->rs2]) ? 1 : 0);
    NEXT();
OP_CMPU:
    c.regs[12] = (c.regs[instr->rs1] < c.regs[instr->rs2]) ? -1 : ((c.regs[instr->rs1] > c.regs[instr->rs2]) ? 1 : 0);
    NEXT();
OP_CMPI:
    c.regs[12] = ((int32_t)c.regs[instr->rs1] < instr->imm) ? -1 : (((int32_t)c.regs[instr->rs1] > instr->imm) ? 1 : 0);
    NEXT();
OP_CMPUI:
    c.regs[12] = (c.regs[instr->rs1] < (uint32_t)instr->imm) ? -1 : ((c.regs[instr->rs1] > (uint32_t) instr->imm) ? 1 : 0);
    NEXT();
OP_TEST:
    c.regs[12] = ((c.regs[instr->rs1] & c.regs[instr->rs2]) != 0) ? 1 : 0;
    NEXT();
OP_TESTI:
    c.regs[12] = ((c.regs[instr->rs1] & (uint32_t)instr->imm) != 0) ? 1 : 0;
    NEXT();

OP_INC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] + 1;
    NEXT();
OP_DEC:
    c.regs[instr->rd] = (uint32_t)(int64_t)(int32_t)c.regs[instr->rd] - 1;
    NEXT();
OP_NOT:
    c.regs[instr->rd] = ~c.regs[instr->rs1];
    NEXT();
OP_ABS:
    if ((int32_t)c.regs[instr->rs1] < 0) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs1];
    NEXT();
OP_NEG:
    if ((int32_t)c.regs[instr->rs1] != INT32_MIN) c.regs[instr->rd] = (uint32_t)-(int32_t)c.regs[instr->rs1];
    NEXT();
OP_MIN:
    if (c.regs[instr->rs1] < c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    NEXT();
OP_MAX:
    if (c.regs[instr->rs1] > c.regs[instr->rs2]) c.regs[instr->rd] = c.regs[instr->rs1];
    else c.regs[instr->rd] = c.regs[instr->rs2];
    NEXT();
OP_MINI:
    if (c.regs[instr->rs1] < instr->imm) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->imm;
    NEXT();
OP_MAXI:
    if (c.regs[instr->rs1] > instr->imm) c.regs[instr->rd] = instr->rs1;
    else c.regs[instr->rd] = instr->imm;
    NEXT();

OP_FADD:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.fregs[instr->rs1]) + std::bit_cast<float>(c.fregs[instr->rs2]));
    NEXT();
OP_FSUB:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.fregs[instr->rs1]) - std::bit_cast<float>(c.fregs[instr->rs2]));
    NEXT();
OP_FMUL:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.fregs[instr->rs1]) * std::bit_cast<float>(c.fregs[instr->rs2]));
    NEXT();
OP_FDIV:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.fregs[instr->rs1]) / std::bit_cast<float>(c.fregs[instr->rs2]));
    NEXT();
OP_FMA:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>( std::fma(std::bit_cast<float>(c.fregs[instr->rd]), std::bit_cast<float>(c.fregs[instr->rs1]), std::bit_cast<float>(c.fregs[instr->rs2])));
    NEXT();
OP_FSQRT:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::sqrt(std::bit_cast<float>(c.fregs[instr->rs1])));
    NEXT();
OP_FABS:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::abs(std::bit_cast<float>(c.fregs[instr->rs1])));
    NEXT();
OP_FNEG:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(-std::bit_cast<float>(c.fregs[instr->rs1]));
    NEXT();
OP_FCMP:
    c.regs[12] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.fregs[instr->rs1]) < std::bit_cast<float>(c.fregs[instr->rs2]) ? -1 : std::bit_cast<float>(c.fregs[instr->rs1]) > std::bit_cast<float>(c.fregs[instr->rs2]) ? 1 : 0);
    NEXT();
OP_ITOF:
    c.fregs[instr->rd] = std::bit_cast<uint32_t>(std::bit_cast<float>(c.regs[instr->rs1]));
    NEXT();
OP_FTOI:
    c.regs[instr->rd] = std::bit_cast<uint32_t>(c.fregs[instr->rs1]);
    NEXT();
OP_FMOV:
    c.fregs[instr->rd] = c.regs[instr->rs1];
    NEXT();
OP_MOVF:
    c.regs[instr->rd] = c.fregs[instr->rs1];
    NEXT();
OP_FLDW_ABS:
    c.fregs[instr->rd] = c.load32(instr->imm);
    NEXT();
OP_FSDW_ABS:
    c.store32(instr->imm, c.fregs[instr->rd]);
    NEXT();
OP_FLDW_BASE:
    c.fregs[instr->rd] = c.load32(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2));
    NEXT();
OP_FSDW_BASE:
    c.store32(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2), c.fregs[instr->rd]);
    NEXT();
OP_FLDW_REG:
    c.fregs[instr->rd] = c.load32(c.regs[instr->rs1] + c.regs[instr->rs2]);
    NEXT();
OP_FSDW_REG:
    c.store32(c.regs[instr->rs1] + c.regs[instr->rs2], c.fregs[instr->rd]);
    NEXT();

OP_MOV_IMM:
    c.regs[instr->rd] = instr->imm;
    NEXT();
OP_MOV_REG:
    c.regs[instr->rd] = c.regs[instr->rs1];
    NEXT();
OP_LDB_ABS:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(instr->imm));
    NEXT();
OP_LDH_ABS:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(instr->imm));
    NEXT();
OP_LDW_ABS:
    c.regs[instr->rd] = c.load32(instr->imm);
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
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2)));
    NEXT();
OP_LDH_BASE:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2)));
    NEXT();
OP_LDW_BASE:
    c.regs[instr->rd] = c.load32(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2));
    NEXT();
OP_LDB_REG:
    c.regs[instr->rd] = static_cast<int8_t>(c.load8(c.regs[instr->rs1] + c.regs[instr->rs2]));
    NEXT();
OP_LDH_REG:
    c.regs[instr->rd] = static_cast<int16_t>(c.load16(c.regs[instr->rs1] + c.regs[instr->rs2]));
    NEXT();
OP_LDW_REG:
    c.regs[instr->rd] = c.load32(c.regs[instr->rs1] + c.regs[instr->rs2]);
    NEXT();
OP_STB_BASE:
    c.store8(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2), c.regs[instr->rd] & 0xFF);
    NEXT();
OP_STH_BASE:
    c.store16(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2), c.regs[instr->rd] & 0xFFFF);
    NEXT();
OP_STW_BASE:
    c.store32(c.regs[instr->rs1] + static_cast<int8_t>(instr->rs2), c.regs[instr->rd]);
    NEXT();
OP_STB_REG:
    c.store8(c.regs[instr->rs1] + c.regs[instr->rs2], c.regs[instr->rd] & 0xFF);
    NEXT();
OP_STH_REG:
    c.store16(c.regs[instr->rs1] + c.regs[instr->rs2], c.regs[instr->rd] & 0xFFFF);
    NEXT();
OP_STW_REG:
    c.store32(c.regs[instr->rs1] + c.regs[instr->rs2], c.regs[instr->rd]);
    NEXT();

OP_PUSH:
    if (c.SP < 4 || c.SP - 4 < c.stack_limit) { return RunResult::STACK_OVERFLOW; } // trap stack overflow
    c.SP -= 4;
    c.store32(c.SP, c.regs[instr->rs1]);
    NEXT();
OP_POP:
    if (c.SP + 4 > c.bus.ram_size()) { return RunResult::STACK_UNDERFLOW; } // stack underflow
    c.regs[instr->rd] = c.load32(c.SP);
    c.SP += 4;
    NEXT();
OP_LEA:
    c.regs[instr->rd] = c.regs[instr->rs1] + instr->imm;
    NEXT();
OP_LEAB:
    c.regs[instr->rd] = instr->imm;
    NEXT();
OP_SWAP:
    std::swap(c.regs[instr->rd], c.regs[instr->rs1]);
    NEXT();
OP_CLR:
    c.regs[instr->rd] = 0;
    NEXT();
OP_MEMCPY: {
    // Copie octet à octet via c.store8/c.load8 : ça respecte le
    // routage RAM/MMIO du bus au lieu de taper direct dans le vector.
    int32_t len = instr->imm;
    if (len > 0) {
        for (uint32_t idx = 0; idx < (uint32_t)len; ++idx)
            c.store8(c.regs[instr->rd] + idx, c.load8(c.regs[instr->rs1] + idx));
    }
    }
    NEXT();
OP_JMP:
    c.PC += instr->imm;
    CHECK_PC();
    FETCH();
    DISPATCH();
OP_JZ:
    {
    if(c.regs[12] == 0) {
        c.PC += instr->imm;
        CHECK_PC();
        FETCH();
        DISPATCH();
    }
    NEXT();
    }
OP_JNZ:
    {
    if(c.regs[12] != 0) {
        c.PC += instr->imm;
        CHECK_PC();
        FETCH();
        DISPATCH();
    }
    NEXT();
    }
OP_JL:
    {
    if((int32_t)c.regs[12] < 0) {
        c.PC += instr->imm;
        CHECK_PC();
        FETCH();
        DISPATCH();
    }
    NEXT();
    }
OP_JG:
    {
    if((int32_t)c.regs[12] > 0) {
        c.PC += instr->imm;
        CHECK_PC();
        FETCH();
        DISPATCH();
    }
    NEXT();
    }
OP_CALL:
    if (c.SP < 4 || c.SP - 4 < c.stack_limit) { return RunResult::STACK_OVERFLOW; } // trap stack overflow
    c.SP -= 4;
    // NOTE: PC + 1 -> PC + INSTR_SIZE : l'adresse de retour est celle de
    // l'instruction SUIVANTE, en octets, plus l'ancien "+1" ne voulait
    // plus rien dire une fois PC en octets.
    c.store32(c.SP, c.PC + INSTR_SIZE);
    c.PC += instr->imm;
    CHECK_PC();
    FETCH();
    DISPATCH();
OP_RET:
    if (c.SP + 4 > c.bus.ram_size()) { return RunResult::STACK_UNDERFLOW; } // stack underflow
    c.PC = c.load32(c.SP);
    c.SP += 4;
    CHECK_PC();
    FETCH();
    DISPATCH();

OP_SYSCALL:
    if (c.SP < 4) return RunResult::STACK_OVERFLOW;
    c.SP -= 4;
    {
        const uint32_t saved = (c.PC + INSTR_SIZE) | (c.mode == PrivMode::USER ? 1u : 0u);
        c.store32(c.SP, saved);
    }
    c.regs[11] = 0;
    c.mode = PrivMode::KERNEL;
    c.PC = c.trap_vector;
    CHECK_PC();
    FETCH();
    DISPATCH();
OP_HALT:
    if (c.mode == PrivMode::USER) {
        c.pending_fault = true;
        c.fault_cause = 2;
        NEXT();
    }
    return RunResult::HALTED;

OP_SETTV:
    //kernel manages traps !
    if (c.mode == PrivMode::KERNEL) {
        c.trap_vector = c.regs[instr->rs1];
    }
    else {
        c.pending_fault = true;
        c.fault_cause = 2;
    }
    NEXT();
OP_SYSRET:
    if (c.SP + 4 > c.bus.ram_size()) return RunResult::STACK_UNDERFLOW;
    {
        const uint32_t saved = c.load32(c.SP);
        c.SP += 4;
        c.mode = (saved & 1u) ? PrivMode::USER : PrivMode::KERNEL;
        c.PC = saved & ~1u;
    }
    CHECK_PC();
    FETCH();
    DISPATCH();
}

#endif