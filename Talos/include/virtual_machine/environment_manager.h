#ifndef ERGON_ENV_MANAGER_H
#define ERGON_ENV_MANAGER_H

#include <chrono>
#include <fstream>
#include <thread>

#include "../hardware/mother_board.h"
#include "../hardware/instructions_handler/step_handler.h"
#include "../hardware/instructions_handler/run_handler.h"
#include "../software/assembler/assembler.h"
#include "../software/assembler/linker.h"

struct StepInfo {
    std::array<uint32_t, 16> regs;
    std::array<uint32_t, 16> fregs;
    uint32_t& PC = regs[15];
    uint32_t& SP = regs[14];
    uint32_t& FP = regs[13];

    DecodedInstr instr;
    PrivMode mode = PrivMode::KERNEL;
    uint32_t trap_vector = 0;

    StepInfo() = default;
    StepInfo(const MotherBoard& mb) {
        regs = mb.cpu->core.regs;
        fregs = mb.cpu->core.fregs;
        PC = mb.cpu->core.PC;
        SP = mb.cpu->core.SP;
        FP = mb.cpu->core.FP;
        mode = mb.cpu->core.mode;
        trap_vector = mb.cpu->core.trap_vector;

        instr = mb.cpu->core.bus.fetch_instr(mb.cpu->core.PC);
    }
};


struct EnvironmentManager {
    size_t ram_size = 0xffffff - 1; // 2^16 - 1
    uint32_t default_stack = 0xfff;
    MotherBoard mb;
    Assembler decoder;
    int exit_code = 1;
    std::atomic<bool> running = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;

    EnvironmentManager(size_t ram_size = 0xffffff - 1, uint32_t stack_size = 0xfff, const std::string& path_to_hard_drive = "") :
     ram_size(ram_size), mb(MotherBoard(ram_size, stack_size)) {
        default_stack = stack_size;
        if (!path_to_hard_drive.empty()) {
            std::ifstream file(path_to_hard_drive, std::ios::binary | std::ios::ate);

            if (!file)
                throw std::runtime_error("Cannot open file: " + path_to_hard_drive);

            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            mb.hard_drive.resize(size);

            if (!file.read(reinterpret_cast<char*>(mb.hard_drive.data()), size))
                throw std::runtime_error("Cannot read file: " + path_to_hard_drive);
        }
    }

    std::string handle_error(const std::string& file_name, const ErrorInfo& e) const {
        const bool located = !e.file.empty();
        std::string e_msg = "Error " + std::to_string(static_cast<int>(e.code));
        if (located) e_msg += " at line " + std::to_string(e.index_line + 1) + " in file " + e.file;
        else e_msg += " in " + file_name;
        e_msg += ": \n" + e.message + "\n\n";

        if (located && decoder.includes) {
            for (const auto& [name, text] : *decoder.includes) {
                if (name != e.file) continue;
                const auto src = PreProcesser::split_lines(text);
                if (e.index_line < src.size()) {
                    e_msg += src[e.index_line] + "\n";
                    e_msg += std::string(src[e.index_line].size(), '^') + "\n";
                }
                break;
            }
        }
        return e_msg;
    }

    // memory: [ text ][ data ][ rodata ][ bss ][ stack ]
    ErrorCode load_ram(const std::vector<uint8_t>& text, const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& rodata, const uint32_t bss_size) {
        if (text.size() + data.size() + rodata.size() + bss_size > mb.cpu->core.stack_limit)
            return ErrorCode::RAM_OVERFLOW;

        for (size_t i = 0; i < text.size(); i++) {
            if (i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[i] = text[i];
        }
        const size_t data_start = text.size();
        for (size_t i = 0; i < data.size(); i++) {
            if (data_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[data_start + i] = data[i];
        }
        const size_t rodata_start = data_start + data.size();
        for (size_t i = 0; i < rodata.size(); i++) {
            if (rodata_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[rodata_start + i] = rodata[i];
        }
        const size_t bss_start = rodata_start + rodata.size();
        for (size_t i = 0; i < bss_size; i++) {
            if (bss_start + i >= mb.ram.size()) return ErrorCode::RAM_OVERFLOW;
            mb.ram[bss_start + i] = 0;
        }
        return ErrorCode::OK;
    }

    std::string build_single(const std::string& program) {
        return build({ { "main" , program } });
    }

    //args are { { <name/path>, <my_program> } }, returns error message
    std::string build(const std::vector<std::pair<std::string, std::string>>& inputs) {
        std::vector<ObjectFile> obj_files;
        decoder.includes = &inputs;
        for (const auto& [name, program] : inputs) {
            auto [obj_file, error_info] = decoder.decode(name, program);
            if (error_info.code != ErrorCode::OK) return handle_error(name, error_info);
            obj_files.emplace_back(obj_file);
        }
        auto [e, linked_bin] = link(obj_files);
        if (e.code != ErrorCode::OK) return handle_error("linked binary", e);

        mb.reset();
        try {
            mb.set_stack_size(linked_bin.stack_size ? linked_bin.stack_size : default_stack);
        } catch (const std::invalid_argument& ex) {
            return std::string("Error: ") + ex.what() + "\n";
        }

        if (const std::vector<uint8_t> text_bytes = to_bytes(linked_bin.text); load_ram(text_bytes, linked_bin.data, linked_bin.rodata, linked_bin.bss_size) != ErrorCode::OK)
            return handle_error("linked binary", ErrorInfo(ErrorCode::RAM_OVERFLOW, 0));

        mb.cpu->core.PC = linked_bin.entry_pc; // déjà une adresse en octets (voir linker.h)
        return "";
    }

    int get_from_ram(const size_t addr) const {
        if (addr < mb.ram.size())
            return mb.ram[addr];
        return 0;
    }

    uint32_t get_from_reg(const std::string& reg_name) const {
        auto [e, reg_index] = parse_reg(reg_name);
        if (e.code != ErrorCode::OK) return 0;

        if (reg_index < mb.cpu->core.regs.size()) {
            if (reg_name[0] == 'f') return mb.cpu->core.fregs[reg_index];
            return mb.cpu->core.regs[reg_index];
        }
        return 0;
    }

    RunResult start() {
        running = true;
        exit_code = 1;
        start_time = std::chrono::system_clock::now();
        const RunResult RR = run(mb.cpu->core);
        //le kernel place le code de sortie dans r0 avant HALT.
        exit_code = static_cast<int>(mb.cpu->core.regs[0]);
        running = false;
        return RR;
    }

    static std::vector<uint8_t> serialize(const LinkedBinary& lb) {
        std::vector<uint8_t> image = to_bytes(lb.text);
        image.insert(image.end(), lb.data.begin(), lb.data.end());
        image.insert(image.end(), lb.rodata.begin(), lb.rodata.end());
        image.resize(image.size() + lb.bss_size, 0);
        return image;
    }

    std::string build_rom(const std::vector<std::pair<std::string, std::string>>& inputs) {
        std::vector<ObjectFile> obj_files;
        decoder.includes = &inputs;
        for (const auto& [name, program] : inputs) {
            auto [obj_file, error_info] = decoder.decode(name, program);
            if (error_info.code != ErrorCode::OK) return handle_error(name, error_info);
            obj_files.emplace_back(obj_file);
        }
        auto [e, linked_bin] = link(obj_files, ROM_BASE);
        if (e.code != ErrorCode::OK) return handle_error("rom binary", e);

        mb.load_rom(serialize(linked_bin), linked_bin.entry_pc);
        return "";
    }

    static void put32(std::vector<uint8_t>& v, const size_t off, const uint32_t x) {
        for (int i = 0; i < 4; i++)
            v[off + i] = x >> (8 * i) & 0xFF;
    }

    // [header 512 o][image paddée][table de relocalisation paddée]. Liée à USER_BASE. Le bss n'est PAS dans l'image.
    std::string build_executable(const std::vector<std::pair<std::string, std::string>>& inputs,
        std::vector<uint8_t>& out_blocks, const uint32_t fallback_stack = 0x4000) {
        constexpr uint32_t B = StorageDevice::BLOCK_SIZE;
        std::vector<ObjectFile> objs;
        decoder.includes = &inputs;
        for (const auto& [name, program] : inputs) {
            auto [obj, err] = decoder.decode(name, program);
            if (err.code != ErrorCode::OK) return handle_error(name, err);
            objs.emplace_back(obj);
        }
        auto [e, lb] = link(objs, USER_BASE);
        if (e.code != ErrorCode::OK) return handle_error("executable", e);

        std::vector<uint8_t> image = to_bytes(lb.text);
        image.insert(image.end(), lb.data.begin(), lb.data.end());
        image.insert(image.end(), lb.rodata.begin(), lb.rodata.end());
        const auto image_size = static_cast<uint32_t>(image.size());
        image.resize((image.size() + B - 1) / B * B, 0);

        std::vector<uint8_t> relocs(lb.abs_relocs.size() * 4);
        for (size_t i = 0; i < lb.abs_relocs.size(); i++)
            put32(relocs, i * 4, lb.abs_relocs[i]);
        relocs.resize((relocs.size() + B - 1) / B * B, 0);

        std::vector<uint8_t> header(B, 0);
        put32(header, 0, EXEC_MAGIC);
        put32(header, 4, USER_BASE);
        put32(header, 8, lb.entry_pc);
        put32(header, 12, image_size);
        put32(header, 16, lb.bss_size);
        put32(header, 20, lb.stack_size ? lb.stack_size : fallback_stack);
        put32(header, 24, static_cast<uint32_t>(lb.abs_relocs.size()));

        out_blocks = header;
        out_blocks.insert(out_blocks.end(), image.begin(), image.end());
        out_blocks.insert(out_blocks.end(), relocs.begin(), relocs.end());
        return "";
    }

        // Construit tout le disque : superblock, table, kernel, programmes.
    std::string build_disk(const std::vector<std::pair<std::string, std::string>>& kernel_sources,
        const std::vector<ProgramSource>& programs, const uint32_t load_address = KERNEL_BASE) {
        constexpr uint32_t B = StorageDevice::BLOCK_SIZE;
        constexpr uint32_t ENTRY = 32;
        if (programs.size() > B / ENTRY) return "too many programs (max 16)\n";

        std::vector<uint8_t> k_image;
        uint32_t k_entry = 0;
        if (std::string err = build_image(kernel_sources, load_address, k_image, k_entry); !err.empty()) return err;
        k_image.resize((k_image.size() + B - 1) / B * B, 0);
        if (static_cast<uint64_t>(load_address) + k_image.size() > KERNEL_STACK_TOP - 0x1000)
            return "kernel image too large\n";
        const auto k_blocks = static_cast<uint32_t>(k_image.size() / B);

        std::vector<uint8_t> disk(2 * B, 0); //bloc 0 = superblock, bloc 1 = table
        disk.insert(disk.end(), k_image.begin(), k_image.end());

        for (size_t i = 0; i < programs.size(); i++) {
            std::vector<uint8_t> exe;
            if (std::string err = build_executable(programs[i].files, exe); !err.empty()) return err;
            const size_t t = B + i * ENTRY;
            const std::string& n = programs[i].name;
            for (size_t k = 0; k < n.size() && k < 15; k++) disk[t + k] = static_cast<uint8_t>(n[k]);
            put32(disk, t + 16, static_cast<uint32_t>(disk.size() / B)); //header bloc
            put32(disk, t + 20, static_cast<uint32_t>(exe.size() / B));
            disk.insert(disk.end(), exe.begin(), exe.end());
        }

        put32(disk, 0, BOOT_MAGIC);
        put32(disk, 4, DISK_VERSION);
        put32(disk, 8, B);
        put32(disk, 12, static_cast<uint32_t>(disk.size() / B));
        put32(disk, 16, 2); // kernel_start
        put32(disk, 20, k_blocks);
        put32(disk, 24, load_address);
        put32(disk, 28, k_entry);
        put32(disk, 32, 1); // table_start
        put32(disk, 36, static_cast<uint32_t>(programs.size()));

        mb.hard_drive = std::move(disk);
        return "";
    }

    std::string build_image(const std::vector<std::pair<std::string, std::string>>& inputs, uint32_t load_address,
        std::vector<uint8_t>& out_image, uint32_t& out_entry_pc) {
        std::vector<ObjectFile> obj_files;
        decoder.includes = &inputs;
        for (const auto& [name, program] : inputs) {
            auto [obj_file, error_info] = decoder.decode(name, program);
            if (error_info.code != ErrorCode::OK) return handle_error(name, error_info);
            obj_files.emplace_back(obj_file);
        }
        auto [e, linked_bin] = link(obj_files, load_address);
        if (e.code != ErrorCode::OK) return handle_error("image binary", e);

        out_image = serialize(linked_bin);
        out_entry_pc = linked_bin.entry_pc;
        return "";
    }

    void flash_disk(const std::vector<uint8_t>& image, uint32_t start_block) {
        const size_t offset = static_cast<size_t>(start_block) * StorageDevice::BLOCK_SIZE;
        if (offset + image.size() > mb.hard_drive.size())
            mb.hard_drive.resize(offset + image.size(), 0);
        std::ranges::copy(image, mb.hard_drive.begin() + offset);
    }

    // StepInfo step() {
    //     if (mb.cpu->core.PC == 0) {
    //         running = true;
    //         exit_code = 1;
    //         start_time = std::chrono::system_clock::now();
    //     }
    //     if (mb.cpu->core.PC >= mb.ram.size()) return { };
    //     const DecodedInstr current = mb.cpu->core.bus.fetch_instr(mb.cpu->core.PC);
    //     step_instr(mb.cpu->core, current, [this]() { handle_syscall(); return exit_code; });
    //
    //     return { mb };
    // }

};


#endif