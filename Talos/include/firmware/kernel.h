#ifndef ERGON_KERNEL_H
#define ERGON_KERNEL_H


#include "syscall_handler.h"


struct Kernel {
    SyscallHandler syscall_handler;

    Kernel() {

    }



    int sys_exit() {
        return 0;
    }

    int sys_write(SyscallContext& SC) {
        uint32_t addr = SC.r1;
        uint32_t size = SC.r2;
        if (addr > SC.ram.size() || size > SC.ram.size() - addr) {
            return -1;
        }
        uint8_t* data = &SC.ram[addr];

        std::cout.write(reinterpret_cast<const char*>(data), size);
        return 1;
    }

    int sys_read(SyscallContext& SC) {
        const uint32_t addr = SC.r1;
        const uint32_t size = SC.r2;
        if (addr > SC.ram.size() || size > SC.ram.size() - addr) {
            return -1;
        }
        char* data = reinterpret_cast<char*>(&SC.ram[addr]);

        std::streamsize n = 0;
        std::cin.read(data, size);
        n = std::cin.gcount();
        if (std::cin.eof()) std::cin.clear();

        SC.r0 = static_cast<uint32_t>(n);
        return 1;
    }

    int sys_clock(SyscallContext& SC) {
        const auto now = std::chrono::system_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        //QUI CONNAIT START_TIME ? -> APPELER KERNEL

        SC.r0 = static_cast<uint32_t>(elapsed);
        return 1;
    }

    int sys_time(SyscallContext& SC) {
        const auto now = std::chrono::system_clock::now();
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        SC.r0 = static_cast<uint32_t>(timestamp);
        return 1;
    }

};

#endif