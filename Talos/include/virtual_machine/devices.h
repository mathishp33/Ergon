#ifndef ERGON_DEVICES_H
#define ERGON_DEVICES_H


#include <chrono>
#include <cstdint>
#include <iostream>




struct ConsoleDevice {
    static uint32_t write(const uint8_t* data, const size_t size) {
        std::cout.write(reinterpret_cast<const char*>(data), size);
        return size;
    }

    static uint32_t read(uint8_t* data, const size_t size) {
        std::streamsize n = 0;
        std::cin.read(reinterpret_cast<char*>(data), size);
        n = std::cin.gcount();
        if (std::cin.eof()) std::cin.clear();
        return n;
    }
};

struct StorageDevice {

};

struct TimerDevice {
    const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

    uint32_t sys_clock() const {
        const auto now = std::chrono::system_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

        return static_cast<uint32_t>(elapsed);
    }

    static uint32_t sys_time() {
        const auto now = std::chrono::system_clock::now();
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        return static_cast<uint32_t>(timestamp);
    }
};

struct InterruptController {
    
};

struct MMIO {
    ConsoleDevice console;
    StorageDevice storage;
    TimerDevice timer;



};

#endif