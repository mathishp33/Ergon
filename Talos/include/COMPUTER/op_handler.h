#ifndef ERGON_OP_HANDLER_H
#define ERGON_OP_HANDLER_H

#include "core.h"

using Handler = void(*)(SimpleCore&, const DecodedInstr&);

static void op_add(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::ADD, i.rd, i.rs1, i.rs2);
}
static void op_sub(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::SUB, i.rd, i.rs1, i.rs2);
}
static void op_mul(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::MUL, i.rd, i.rs1, i.rs2);
}
static void op_div(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::DIV, i.rd, i.rs1, i.rs2);
}
static void op_mod(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::MOD, i.rd, i.rs1, i.rs2);
}
static void op_addi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::ADD, i.rd, i.rs1, i.rs2);
}
static void op_subi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::SUB, i.rd, i.rs1, i.rs2);
}
static void op_muli(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::MUL, i.rd, i.rs1, i.rs2);
}
static void op_divi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::DIV, i.rd, i.rs1, i.rs2);
}
static void op_modi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::MOD, i.rd, i.rs1, i.rs2);
}

static void op_and(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::AND, i.rd, i.rs1, i.rs2);
}
static void op_or(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::OR, i.rd, i.rs1, i.rs2);
}
static void op_xor(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::XOR, i.rd, i.rs1, i.rs2);
}
static void op_andi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::AND, i.rd, i.rs1, i.rs2);
}
static void op_ori(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::OR, i.rd, i.rs1, i.rs2);
}
static void op_xori(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::XOR, i.rd, i.rs1, i.rs2);
}

static void op_shl(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::SHL, i.rd, i.rs1, i.rs2);
}
static void op_shr(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::SHR, i.rd, i.rs1, i.rs2);
}
static void op_sar(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::SAR, i.rd, i.rs1, i.rs2);
}
static void op_rol(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::ROL, i.rd, i.rs1, i.rs2);
}
static void op_ror(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::ROR, i.rd, i.rs1, i.rs2);
}
static void op_shli(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::SHL, i.rd, i.rs1, i.rs2);
}
static void op_shri(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::SHR, i.rd, i.rs1, i.rs2);
}
static void op_sari(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::SAR, i.rd, i.rs1, i.rs2);
}
static void op_roli(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::ROL, i.rd, i.rs1, i.rs2);
}
static void op_rori(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::ROR, i.rd, i.rs1, i.rs2);
}

static void op_cmp(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::CMP, 0, i.rs1, i.rs2);
}
static void op_cmpu(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::CMPU, 0, i.rs1, i.rs2);
}
static void op_test(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::TEST, 0, i.rs1, i.rs2);
}
static void op_cmpi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::CMP, 0, i.rs1, i.rs2);
}
static void op_cmpui(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::CMPU, 0, i.rs1, i.rs2);
}
static void op_testi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::TEST, 0, i.rs1, i.rs2);
}

static void op_inc(SimpleCore& c, const DecodedInstr& i) {
    c.J_type_instr(ALUOp::INC, i.rd);
}
static void op_dec(SimpleCore& c, const DecodedInstr& i) {
    c.J_type_instr(ALUOp::DEC, i.rd);
}
static void op_not(SimpleCore& c, const DecodedInstr& i) {
    c.J_type_instr(ALUOp::NOT, i.rd, i.rs1);
}
static void op_abs(SimpleCore& c, const DecodedInstr& i) {
    c.J_type_instr(ALUOp::ABS, i.rd, i.rs1);
}
static void op_neg(SimpleCore& c, const DecodedInstr& i) {
    c.J_type_instr(ALUOp::NEG, i.rd, i.rs1);
}
static void op_min(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::MIN, i.rd, i.rs1, i.rs2);
}
static void op_max(SimpleCore& c, const DecodedInstr& i) {
    c.R_type_instr(ALUOp::MAX, i.rd, i.rs1, i.rs2);
}
static void op_mini(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::MIN, i.rd, i.rs1, i.rs2);
}
static void op_maxi(SimpleCore& c, const DecodedInstr& i) {
    c.I_type_instr(ALUOp::MAX, i.rd, i.rs1, i.rs2);
}

static void op_mov_imm(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = i.imm;
    c.update_flags(i.rd);
}
static void op_mov_reg(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = c.regs[i.rs1];
}
static void op_ldb_abs(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = static_cast<int8_t>(c.load8(i.imm));
    c.update_flags(i.rd);
}
static void op_ldh_abs(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = static_cast<int16_t>(c.load16(i.imm));
    c.update_flags(i.rd);
}
static void op_ldw_abs(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = c.load32(i.imm);
    c.update_flags(i.rd);
}
static void op_stb_abs(SimpleCore& c, const DecodedInstr& i) {
    c.store8(i.imm, c.regs[i.rd] & 0xFF);
}
static void op_sth_abs(SimpleCore& c, const DecodedInstr& i) {
    c.store16(i.imm, c.regs[i.rd] & 0xFFFF);
}
static void op_stw_abs(SimpleCore& c, const DecodedInstr& i) {
    c.store32(i.imm, c.regs[i.rd]);
}
static void op_ldb_base(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = static_cast<int8_t>(c.load8(c.regs[i.rs1] + static_cast<int8_t>(i.imm)));
    c.update_flags(i.rd);
}
static void op_ldh_base(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = static_cast<int16_t>(c.load16(c.regs[i.rs1] + static_cast<int8_t>(i.imm)));
    c.update_flags(i.rd);
}
static void op_ldw_base(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = c.load32(c.regs[i.rs1] + static_cast<int8_t>(i.imm));
    c.update_flags(i.rd);
}
static void op_stb_base(SimpleCore& c, const DecodedInstr& i) {
    c.store8(c.regs[i.rs1] + static_cast<int8_t>(i.imm), c.regs[i.rd] & 0xFF);
}
static void op_sth_base(SimpleCore& c, const DecodedInstr& i) {
    c.store16(c.regs[i.rs1] + static_cast<int8_t>(i.imm), c.regs[i.rd] & 0xFFFF);
}
static void op_stw_base(SimpleCore& c, const DecodedInstr& i) {
    c.store32(c.regs[i.rs1] + static_cast<int8_t>(i.imm), c.regs[i.rd]);
}

static void op_push(SimpleCore& c, const DecodedInstr& i) {
    c.SP -= 4;
    c.store32(c.SP, c.regs[i.rs1]);
}
static void op_pop(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = c.load32(c.SP);
    c.update_flags(i.rd);
    c.SP += 4;
}
// Load Effective Address
static void op_lea(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = c.regs[i.rs1] + static_cast<int8_t>(i.imm);
    c.update_flags(i.rd);
}
// Swap registers
static void op_swap(SimpleCore& c, const DecodedInstr& i) {
    std::swap(c.regs[i.rd], c.regs[i.rs1]);
}
// Clear register
static void op_clr(SimpleCore& c, const DecodedInstr& i) {
    c.regs[i.rd] = 0;
    c.flags.Z = true;
    c.flags.N = false;
}
// Memory copy (rd = destination, rs1 = source, imm = length)
static void op_memcpy(SimpleCore& c, const DecodedInstr& i) {
    int32_t len = static_cast<int8_t>(i.imm);
    if (len < 0) return;
    for (uint32_t index = 0; index < i.rs2; ++index)
        if (c.regs[i.rd] + index < c.ram.size() && c.regs[i.rs1] + index < c.ram.size())
            c.ram[c.regs[i.rd] + index] = c.ram[c.regs[i.rs1] + index];
}

static void op_jmp(SimpleCore& c, const DecodedInstr& i) {
    c.PC += i.imm;
    c.inc_pc = false;
}
static void op_jz(SimpleCore& c, const DecodedInstr& i) {
    if (c.flags.Z) {
        c.PC += i.imm;
        c.inc_pc = false;
    }
}
static void op_jnz(SimpleCore& c, const DecodedInstr& i) {
    if (!c.flags.Z) {
        c.PC += i.imm;
        c.inc_pc = false;
    }
}
static void op_jg(SimpleCore& c, const DecodedInstr& i) {
    if (!c.flags.Z && !c.flags.N) {
        c.PC += i.imm;
        c.inc_pc = false;
    }
}
static void op_jl(SimpleCore& c, const DecodedInstr& i) {
    if (c.flags.N) {
        c.PC += i.imm;
        c.inc_pc = false;
    }
}
static void op_call(SimpleCore& c, const DecodedInstr& i) {
    c.SP -= 4;
    c.store32(c.SP, c.PC + 1);
    c.PC += i.imm;
    c.inc_pc = false;
}
static void op_ret(SimpleCore& c, const DecodedInstr& i) {
    c.PC = c.load32(c.SP);
    c.SP += 4;
    c.inc_pc = false;
}

static void op_halt(SimpleCore& c, const DecodedInstr&) {
    c.PC = 0xFFFFFFFF;
    c.inc_pc = false;
}


static inline Handler dispatch[256] = {
    /* 0x00 */ op_add,
    /* 0x01 */ op_sub,
    /* 0x02 */ op_mul,
    /* 0x03 */ op_div,
    /* 0x04 */ op_mod,
    /* 0x05 */ op_addi,
    /* 0x06 */ op_subi,
    /* 0x07 */ op_muli,
    /* 0x08 */ op_divi,
    /* 0x09 */ op_modi,

    /* 0x0A */ op_and,
    /* 0x0B */ op_or,
    /* 0x0C */ op_xor,
    /* 0x0D */ op_andi,
    /* 0x0E */ op_ori,
    /* 0x0F */ op_xori,

    /* 0x10 */ op_shl,
    /* 0x11 */ op_shr,
    /* 0x12 */ op_sar,
    /* 0x13 */ op_rol,
    /* 0x14 */ op_ror,
    /* 0x15 */ op_shli,
    /* 0x16 */ op_shri,
    /* 0x17 */ op_sari,
    /* 0x18 */ op_roli,
    /* 0x19 */ op_rori,

    /* 0x1A */ op_cmp,
    /* 0x1B */ op_cmpu,
    /* 0x1C */ op_test,
    /* 0x1D */ op_cmpi,
    /* 0x1E */ op_cmpui,
    /* 0x1F */ op_testi,

    /* 0x20 */ op_inc,
    /* 0x21 */ op_dec,
    /* 0x22 */ op_not,
    /* 0x23 */ op_abs,
    /* 0x24 */ op_neg,
    /* 0x25 */ op_min,
    /* 0x26 */ op_max,
    /* 0x27 */ op_mini,
    /* 0x28 */ op_maxi,

    /* FPU gap */
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

    /* 0x30 */ op_mov_imm,
    /* 0x31 */ op_mov_reg,
    /* 0x32 */ op_ldb_abs,
    /* 0x33 */ op_ldh_abs,
    /* 0x34 */ op_ldw_abs,
    /* 0x35 */ op_stb_abs,
    /* 0x36 */ op_sth_abs,
    /* 0x37 */ op_stw_abs,
    /* 0x38 */ op_ldb_base,
    /* 0x39 */ op_ldh_base,
    /* 0x3A */ op_ldw_base,
    /* 0x3B */ op_stb_base,
    /* 0x3C */ op_sth_base,
    /* 0x3D */ op_stw_base,
    /* 0x3E */ op_push,
    /* 0x3F */ op_pop,
    /* 0x40 */ op_lea,
    /* 0x41 */ op_swap,
    /* 0x42 */ op_clr,
    /* 0x43 */ op_memcpy,

    /* 0x44 */ op_jmp,
    /* 0x45 */ op_jz,
    /* 0x46 */ op_jnz,
    /* 0x47 */ op_jg,
    /* 0x48 */ op_jl,
    /* 0x49 */ op_call,
    /* 0x4A */ op_ret,

    /* 0x4B */ op_halt
};


inline void step_instr(SimpleCore& c, const DecodedInstr& i) {
    c.inc_pc = true;
    dispatch[i.opcode](c, i);
    if (c.inc_pc)
        c.PC++;
}


#endif