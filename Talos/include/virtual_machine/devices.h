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
    // TODO: le stockage persistant se prête mal à un simple registre
    // MMIO (transferts par bloc). Idée : garder ça côté syscall
    // (comme aujourd'hui dans EnvironmentManager::handle_syscall),
    // et réserver le MMIO pour un petit registre de "status"
    // (prêt/occupé/erreur) que le kernel peut poller.
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
    // TODO: registre de masque + registre de "pending" en MMIO.
    // Quand un device veut lever une IRQ (ex: donnée dispo sur la
    // console), il pose un bit ici ; le CORE (ou run_handler) check
    // ce champ à chaque NEXT() si tu veux des interruptions plutôt
    // que du polling pur.
};

// ------------------------------------------------------------------
// MMIO : table de registres exposée sur le bus, offsets relatifs à
// MMIO_BASE (voir bus.h). C'est le point d'extension pour ajouter
// des devices : chaque device se voit attribuer une plage d'offsets.
// ------------------------------------------------------------------
struct MMIO {
    ConsoleDevice console;
    StorageDevice storage;
    TimerDevice timer;
    InterruptController irq;

    enum Offset : uint32_t {
        CONSOLE_OUT  = 0x0000, // store8 : écrit un octet sur stdout
        CONSOLE_IN   = 0x0004, // load8 : lit un octet depuis stdin (0 si rien)
        TIMER_CLOCK  = 0x0010, // load32 : ms depuis le démarrage de la VM
        TIMER_TIME   = 0x0014, // load32 : timestamp unix
        // STORAGE   = 0x0100..
        // IRQ       = 0x0200..
    };

    uint8_t load8(uint32_t off) {
        switch (off) {
            case CONSOLE_IN: {
                uint8_t b = 0;
                ConsoleDevice::read(&b, 1);
                return b;
            }
            default: return 0;
        }
    }
    void store8(uint32_t off, uint8_t v) {
        switch (off) {
            case CONSOLE_OUT: ConsoleDevice::write(&v, 1); break;
            default: break;
        }
    }


    uint16_t load16(uint32_t off) { return load8(off); }
    void store16(uint32_t off, uint16_t v) { store8(off, static_cast<uint8_t>(v)); }

    uint32_t load32(uint32_t off) {
        switch (off) {
            case TIMER_CLOCK:
                return timer.sys_clock();
            case TIMER_TIME:
                return TimerDevice::sys_time();
            default:
                return load8(off);
        }
    }
    void store32(uint32_t off, uint32_t v) { store8(off, static_cast<uint8_t>(v)); }
};

#endif