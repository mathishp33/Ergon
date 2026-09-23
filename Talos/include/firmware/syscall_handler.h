#ifndef ERGON_SYSCALL_HANDLER_H
#define ERGON_SYSCALL_HANDLER_H


#include <chrono>
#include <iostream>

#include "hardware/mother_board.h"


enum SYSCALL : uint32_t {
    EXIT = 0,
    WRITE = 1,
    READ = 2,
    CLOCK = 3,
    TIME = 4,
    DISK_READ = 5,
    DISK_WRITE = 6,
};


struct SyscallContext {
    uint32_t r0; //syscall
    uint32_t& r1; //arg0
    uint32_t& r2; //arg1
    uint32_t& r3; //arg2
    std::vector<uint8_t>& ram;

    SyscallContext(uint32_t r0_, uint32_t& r1_, uint32_t& r2_, uint32_t& r3_, std::vector<uint8_t>& ram_) :
    r0(r0_), r1(r1_), r2(r2_), r3(r3_), ram(ram_) {}
};

struct SyscallHandler {
    //&regs[0], &regs[1], &regs[2], &regs[3], std::shared_ptr RAM
    SyscallHandler() {}

    int handle_syscall(SyscallContext SC) {
        switch (SC.r0) {
        case EXIT:
            return sys_exit();
        case WRITE:
            return sys_write(SC);
        case READ:
            return sys_read(SC);
        case CLOCK:
            return sys_clock(SC);
        case TIME:
            return sys_time(SC);
        case DISK_READ:
            break;
        case DISK_WRITE:
            break;
        default:
            break;
        }
        return 1;
    }



};



#endif