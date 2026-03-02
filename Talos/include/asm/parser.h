#ifndef ERGON_PARSER_H
#define ERGON_PARSER_H

#include "error.h"
#include "utils.h"

#include <unordered_map>


inline std::unordered_map<std::string, uint8_t> reg_table = {
    { "r0", 0 },
    { "r1", 1 },
    { "r2", 2 },
    { "r3", 3 },
    { "r4", 4 },
    { "r5", 5 },
    { "r6", 6 },
    { "r7", 7 },
    { "r8", 8 },
    { "r9", 9 },
    { "r10", 10 },
    { "r11", 11 },
    { "tmp", 12 },
    { "cmp", 13 },
    { "sp", 14 },
    { "zero", 15 }
};

static std::pair<ErrorInfo, uint8_t> parse_reg(const std::string& s) {
    if (reg_table.contains(s))
        return { { }, reg_table[s] };
    return { { ErrorCode::INVALID_REG, "invalid register \"" + s + "\"" }, 0 };
}

static std::pair<ErrorInfo, int> parse_imm(const std::string& s, std::unordered_map<std::string, int32_t>& constants) {
    if (s.starts_with("0x"))
        return string_utils::better_stoi(s.substr(2), nullptr, 16);
    if (s.starts_with("0b"))
        return string_utils::better_stoi(s.substr(2), nullptr, 2);
    if (constants.contains(s))
        return { {}, constants[s] };
    return string_utils::better_stoi(s);
}

// inline std::pair<ErrorInfo, std::vector<uint8_t>> parse_bytes(const std::string& s, size_t final_size) {
//     if (final_size == 0)
//         return { { }, {} };
//
//     std::vector<uint8_t> bytes;
//     bytes.reserve(final_size);
//
//     //BASE 16
//     if (s.starts_with("0x")) {
//         std::string hex = s.substr(2);
//
//         // 2 hex chars = 1 byte
//         if (hex.size() != final_size * 2)
//             return { { ErrorCode::INVALID_BYTE_SIZE, "invalid byte size" }, {} };
//
//         for (size_t i = 0; i < hex.size(); i += 2) {
//             auto [e, value] = string_utils::better_stoi(hex.substr(i, 2), nullptr, 16);
//             if (e.code != ErrorCode::OK) return { e, {} };
//
//             if (value > 0xFF) return { { ErrorCode::INVALID_BYTE_SIZE, "invalid byte size" }, {} };
//
//             bytes.push_back(static_cast<uint8_t>(value));
//         }
//         return { { }, bytes };
//     }
//
//     //BASE 2
//     if (s.starts_with("0b")) {
//         std::string bin = s.substr(2);
//
//         // 8 bits = 1 byte
//         if (bin.size() != final_size * 8) return { { ErrorCode::INVALID_BYTE_SIZE, "invalid byte size" }, {} };
//
//         for (size_t i = 0; i < bin.size(); i += 8) {
//             auto [e, value] = string_utils::better_stoi(bin.substr(i, 8), nullptr, 2);
//             if (e.code != ErrorCode::OK) return { e, {} };
//
//             if (value > 0xFF) return { { ErrorCode::INVALID_BYTE_SIZE, "invalid byte size" }, {} };
//
//             bytes.push_back(static_cast<uint8_t>(value));
//         }
//         return { {}, bytes };
//     }
//
//     //STRING / CHAR LITERAL
//     if ((s.starts_with("\"") && s.ends_with("\"")) || (s.starts_with("'")  && s.ends_with("'"))) {
//         std::string content = s.substr(1, s.size() - 2);
//
//         if (content.size() != final_size) return { { ErrorCode::INVALID_BYTE_SIZE, "invalid byte size" }, {} };
//
//         for (unsigned char c : content)
//             bytes.push_back(static_cast<uint8_t>(c));
//
//         return { { }, bytes };
//     }
//
//     //BASE 10
//     {
//         auto [e, value] = string_utils::better_stoi(s, nullptr, 10);
//         if (e.code != ErrorCode::OK) return { e, {} };
//
//         for (size_t i = 0; i < final_size; ++i)
//             bytes.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
//
//         return { { }, bytes };
//     }
// }


#endif