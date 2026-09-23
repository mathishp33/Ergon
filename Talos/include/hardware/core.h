#ifndef ERGON_CORE_H
#define ERGON_CORE_H

#include "alu.h"
#include "fpu.h"
#include "bus.h"

#include <vector>
#include <array>


enum OPCODE : uint8_t {
    ADD, SUB, MUL, DIV, MOD,
    ADDI, SUBI, MULI, DIVI, MODI,

    AND, OR, XOR, ANDI, ORI, XORI,

    SHL, SHR, SAR, ROL, ROR,
    SHLI, SHRI, SARI, ROLI, RORI,

    CMP, CMPU, TEST,
    CMPI, CMPUI, TESTI,

    INC, DEC, NOT, ABS, NEG,
    MIN, MAX, MINI, MAXI,

    FADD, FSUB, FMUL, FDIV, FMA,
    FSQRT, FABS, FNEG, FCMP, ITOF, FTOI,
    FMOV, MOVF, FLDW_ABS, FSTW_ABS,
    FLDW_BASE, FSTW_BASE, FLDW_REG, FSTW_REG,

    MOV_IMM, MOV_REG,
    LDB_ABS, LDH_ABS, LDW_ABS,
    STB_ABS, STH_ABS, STW_ABS,
    LDB_BASE, LDH_BASE, LDW_BASE,
    LDB_REG, LDH_REG, LDW_REG,
    STB_BASE, STH_BASE, STW_BASE,
    SDB_REG, SDH_REG, SDW_REG,
    PUSH, POP,
    LEA, LEAB, SWAP, CLR, MEMCPY,

    JMP, JZ, JNZ, JG, JL,
    CALL, RET,

    SYSCALL, HALT,

    // trap/privilege
    SETTV,  // settv rs : trap_vector = regs[rs] (kernel-only)
    SYSRET  // sysret : depile (PC | mode) saved and restored by syscall
};


enum class PrivMode : uint8_t {
    KERNEL = 0,
    USER = 1
};

struct SimpleCore {
    std::array<uint32_t, 16> regs{};
    std::array<uint32_t, 16> fregs{};
    uint32_t& FP = regs[13]; // Frame Pointer
    uint32_t& SP = regs[14]; // Stack Pointer
    uint32_t& PC = regs[15]; // Program Counter

    uint32_t stack_limit = 0;

    PrivMode mode = PrivMode::KERNEL;
    uint32_t trap_vector = 0;
    bool pending_fault = false;
    uint32_t fault_cause = 0; // 1 = MMIO forbidden, 2 = privileged
    SystemBus& bus;

    SimpleCore(SystemBus& bus, uint32_t ram_size) : bus(bus) {
        PC = ROM_BASE;
        SP = ram_size;
    }

    void reset(uint32_t ram_size) {
        std::ranges::fill(regs, 0);
        std::ranges::fill(fregs, 0);
        SP = ram_size;
        FP = 0;
        PC = ROM_BASE;
        mode = PrivMode::KERNEL;
        trap_vector = 0;
        pending_fault = false;
        fault_cause = 0;
    }

    uint32_t load32(uint32_t addr) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return 0;
        }
        return bus.load32(addr);
    }
    uint16_t load16(uint32_t addr) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return 0;
        }
        return bus.load16(addr);
    }
    uint8_t load8(uint32_t addr) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return 0;
        }
        return bus.load8(addr);
    }
    void store32(uint32_t addr, uint32_t value) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return;
        }
        bus.store32(addr, value);
    }
    void store16(uint32_t addr, uint16_t value) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return;
        }
        bus.store16(addr, value);
    }
    void store8(uint32_t addr, uint8_t value) {
        if (mode == PrivMode::USER && bus.is_mmio(addr)) {
            pending_fault = true;
            fault_cause = 1;
            return;
        }
        bus.store8(addr, value);
    }
};


#endif