#ifndef ERGON_BUS_H
#define ERGON_BUS_H

#include <cstdint>
#include <vector>

#include "virtual_machine/devices.h"
#include "software/data.h"

// memory:
// [0x00000000, RAM max) -> RAM
// [ROM_BASE,   ROM_BASE + rom.size) -> ROM (read-only)
// [MMIO_BASE,  0xFFFFFFFF] -> MMIO
//

constexpr uint32_t ROM_BASE  = 0x10000000;
constexpr uint32_t MMIO_BASE = 0xF0000000;

struct SystemBus {
    std::vector<uint8_t>& ram;
    std::vector<uint8_t>& rom;
    MMIO& mmio;

    SystemBus(std::vector<uint8_t>& ram, std::vector<uint8_t>& rom, MMIO& mmio) : ram(ram), rom(rom), mmio(mmio) {}

    static bool is_mmio(uint32_t addr) {
        return addr >= MMIO_BASE;
    }
    [[nodiscard]] bool is_rom(uint32_t addr) const {
        return addr >= ROM_BASE && (addr - ROM_BASE) < rom.size();
    }
    [[nodiscard]] size_t ram_size() const {
        return ram.size();
    }

    [[nodiscard]] bool pc_in_bounds(uint32_t pc) const {
        if ((uint64_t)pc + INSTR_SIZE <= ram.size()) return true;
        if (is_rom(pc) && (uint64_t)(pc - ROM_BASE) + INSTR_SIZE <= rom.size()) return true;
        return false;
    }

    uint32_t load32(uint32_t addr) {
        if (is_mmio(addr)) return mmio.load32(addr - MMIO_BASE);
        if (is_rom(addr)) {
            uint32_t off = addr - ROM_BASE;
            if (off + 3 >= rom.size()) return 0;
            return rom[off] | (rom[off + 1] << 8) | (rom[off + 2] << 16) | (rom[off + 3] << 24);
        }
        if (addr + 3 >= ram.size()) return 0;
        return ram[addr] | (ram[addr + 1] << 8) | (ram[addr + 2] << 16) | (ram[addr + 3] << 24);
    }
    uint16_t load16(uint32_t addr) {
        if (is_mmio(addr)) return mmio.load16(addr - MMIO_BASE);
        if (is_rom(addr)) {
            uint32_t off = addr - ROM_BASE;
            if (off + 1 >= rom.size()) return 0;
            return rom[off] | (rom[off + 1] << 8);
        }
        if (addr + 1 >= ram.size()) return 0;
        return ram[addr] | (ram[addr + 1] << 8);
    }
    uint8_t load8(uint32_t addr) {
        if (is_mmio(addr)) return mmio.load8(addr - MMIO_BASE);
        if (is_rom(addr)) {
            uint32_t off = addr - ROM_BASE;
            if (off >= rom.size()) return 0;
            return rom[off];
        }
        if (addr >= ram.size()) return 0;
        return ram[addr];
    }

    void store32(uint32_t addr, uint32_t value) {
        if (is_mmio(addr)) {
            mmio.store32(addr - MMIO_BASE, value);
            return;
        }
        if (is_rom(addr)) return;
        if (addr + 3 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
        ram[addr + 2] = (value >> 16) & 0xFF;
        ram[addr + 3] = (value >> 24) & 0xFF;
    }
    void store16(uint32_t addr, uint16_t value) {
        if (is_mmio(addr)) {
            mmio.store16(addr - MMIO_BASE, value);
            return;
        }
        if (is_rom(addr)) return;
        if (addr + 1 >= ram.size()) return;
        ram[addr] = value & 0xFF;
        ram[addr + 1] = (value >> 8) & 0xFF;
    }
    void store8(uint32_t addr, uint8_t value) {
        if (is_mmio(addr)) {
            mmio.store8(addr - MMIO_BASE, value);
            return;
        }
        if (is_rom(addr)) return;
        if (addr >= ram.size()) return;
        ram[addr] = value;
    }

    // fetch/write Instr -> kernel, loader, user
    DecodedInstr fetch_instr(uint32_t addr) {
        DecodedInstr instr;
        instr.opcode = load8(addr);
        instr.rd = load8(addr + 1);
        instr.rs1 = load8(addr + 2);
        instr.rs2 = load8(addr + 3);
        instr.imm = static_cast<int32_t>(load32(addr + 4));
        return instr;
    }

    void store_instr(uint32_t addr, const DecodedInstr& instr) {
        uint8_t bytes[INSTR_SIZE];
        encode_instr(instr, bytes);
        for (uint32_t i = 0; i < INSTR_SIZE; i++)
            store8(addr + i, bytes[i]);
    }
};

#endif