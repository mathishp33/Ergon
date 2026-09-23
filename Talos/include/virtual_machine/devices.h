#ifndef ERGON_DEVICES_H
#define ERGON_DEVICES_H

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>


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
    static constexpr uint32_t BLOCK_SIZE = 512;

    std::vector<uint8_t>& hard_drive;
    std::vector<uint8_t>& ram;

    uint32_t block = 0;  // numéro de bloc ciblé par la prochaine commande
    uint32_t buffer = 0; // adresse RAM du buffer (BLOCK_SIZE octets)
    uint32_t status = 0; // 0 = dernière opération OK, 1 = erreur (hors limites)

    StorageDevice(std::vector<uint8_t>& hard_drive, std::vector<uint8_t>& ram)
        : hard_drive(hard_drive), ram(ram) {}

    uint32_t block_count() const {
        return static_cast<uint32_t>(hard_drive.size() / BLOCK_SIZE);
    }

    void do_read() {
        const uint64_t disk_off = static_cast<uint64_t>(block) * BLOCK_SIZE;
        if (block >= block_count() || (uint64_t)buffer + BLOCK_SIZE > ram.size()) {
            status = 1;
            return;
        }
        std::copy(hard_drive.begin() + disk_off, hard_drive.begin() + disk_off + BLOCK_SIZE,
                   ram.begin() + buffer);
        status = 0;
    }

    void do_write() {
        const uint64_t disk_off = static_cast<uint64_t>(block) * BLOCK_SIZE;
        if (block >= block_count() || (uint64_t)buffer + BLOCK_SIZE > ram.size()) {
            status = 1;
            return;
        }
        std::copy(ram.begin() + buffer, ram.begin() + buffer + BLOCK_SIZE,
                   hard_drive.begin() + disk_off);
        status = 0;
    }
};

struct TimerDevice {
    const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

    [[nodiscard]] uint32_t sys_clock() const {
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
    // TODO: registre de masque + registre de "pending" en MMIO.

};

//voir bus.h pour nouveau addressages
struct MMIO {
    ConsoleDevice console;
    StorageDevice storage;
    TimerDevice timer;
    InterruptController irq;

    MMIO(std::vector<uint8_t>& hard_drive, std::vector<uint8_t>& ram) : storage(hard_drive, ram) {}

    enum Offset : uint32_t {
        CONSOLE_OUT = 0x0000, // store8 : écrit un octet sur stdout
        CONSOLE_IN = 0x0004, // load8 : lit un octet depuis stdin (0 si rien)
        TIMER_CLOCK = 0x0010, // load32 : ms depuis le démarrage de la VM
        TIMER_TIME = 0x0014, // load32 : timestamp unix

        //BLOCS DE 512 OCTETS
        DISK_BLOCK = 0x0100, // store32 : numéro de bloc pour la prochaine commande
        DISK_BUFFER = 0x0104, // store32 : adresse RAM du buffer (BLOCK_SIZE octets)
        DISK_CMD = 0x0108, // store32 : 1 = lire (disque -> buffer), 2 = écrire (buffer -> disque)
        DISK_STATUS = 0x010C, // load32 : 0 = OK, 1 = erreur (bloc/buffer hors limites)
        DISK_BLOCK_SIZE = 0x0110, // load32 : taille d'un bloc en octets (constante)
        DISK_BLOCK_COUNT = 0x0114, // load32 : nombre total de blocs disponibles
        // IRQ_* = 0x0200.. (c'est un outil mystère qui nous servira plus tard)
    };

    static uint8_t load8(uint32_t off) {
        switch (off) {
            case CONSOLE_IN: {
                uint8_t b = 0;
                ConsoleDevice::read(&b, 1);
                return b;
            }
            default: return 0;
        }
    }
    static void store8(uint32_t off, uint8_t v) {
        switch (off) {
            case CONSOLE_OUT: ConsoleDevice::write(&v, 1); break;
            default: break;
        }
    }


    static uint16_t load16(uint32_t off) { return load8(off); }
    static void store16(uint32_t off, uint16_t v) { store8(off, static_cast<uint8_t>(v)); }

    [[nodiscard]] uint32_t load32(uint32_t off) const {
        switch (off) {
            case TIMER_CLOCK: return timer.sys_clock();
            case TIMER_TIME: return TimerDevice::sys_time();
            case DISK_STATUS: return storage.status;
            case DISK_BLOCK_SIZE: return StorageDevice::BLOCK_SIZE;
            case DISK_BLOCK_COUNT: return storage.block_count();
            default: return load8(off);
        }
    }
    void store32(uint32_t off, uint32_t v) {
        switch (off) {
            case DISK_BLOCK:
                storage.block = v;
                break;
            case DISK_BUFFER:
                storage.buffer = v;
                break;
            case DISK_CMD:
                if (v == 1) storage.do_read();
                else if (v == 2) storage.do_write();
                break;
            default:
                store8(off, static_cast<uint8_t>(v));
                break;
        }
    }
};

#endif