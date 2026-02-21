#ifndef ERGON_PARSER_H
#define ERGON_PARSER_H

#include "error.h"
#include "utils.h"

#include <unordered_map>


inline std::unordered_map<std::string, uint8_t> reg_table = {
    { "zero", 0 },
    { "eax", 1 },
    { "ebx", 2 },
    { "ecx", 3 },
    { "edx", 4 },
    { "esi", 5 },
    { "edi", 6 },
    { "r7", 7 },
    { "r8", 8 },
    { "r9", 9 },
    { "r10", 10 },
    { "r11", 11 },
    { "r12", 12 },
    { "sp", 13 },
    { "bp", 14 },
    { "r15", 15 }
};

static std::pair<ErrorCode, uint8_t> parse_reg(const std::string& s) {
    if (reg_table.contains(s))
        return { ErrorCode::OK, reg_table[s] };
    return { ErrorCode::INVALID_ARG, 0 };
}

static std::pair<ErrorCode, int> parse_imm(const std::string& s) {
    if (s.starts_with("0x"))
        return string_utils::better_stoi(s.substr(2), nullptr, 16);
    if (s.starts_with("0b"))
        return string_utils::better_stoi(s.substr(2), nullptr, 2);
    return string_utils::better_stoi(s);
}

inline std::pair<ErrorCode, std::vector<uint8_t>> parse_bytes(const std::string& s, size_t final_size) {
    if (final_size == 0)
        return { ErrorCode::OK, {} };

    std::vector<uint8_t> bytes;
    bytes.reserve(final_size);

    //BASE 16
    if (s.starts_with("0x")) {
        std::string hex = s.substr(2);

        // 2 hex chars = 1 byte
        if (hex.size() != final_size * 2)
            return { ErrorCode::INVALID_ARG, {} };

        for (size_t i = 0; i < hex.size(); i += 2) {
            auto [ec, value] =
                string_utils::better_stoi(hex.substr(i, 2), nullptr, 16);

            if (ec != ErrorCode::OK || value > 0xFF)
                return { ErrorCode::INVALID_ARG, {} };

            bytes.push_back(static_cast<uint8_t>(value));
        }
        return { ErrorCode::OK, bytes };
    }

    //BASE 2
    if (s.starts_with("0b")) {
        std::string bin = s.substr(2);

        // 8 bits = 1 byte
        if (bin.size() != final_size * 8)
            return { ErrorCode::INVALID_ARG, {} };

        for (size_t i = 0; i < bin.size(); i += 8) {
            auto [ec, value] =
                string_utils::better_stoi(bin.substr(i, 8), nullptr, 2);

            if (ec != ErrorCode::OK || value > 0xFF)
                return { ErrorCode::INVALID_ARG, {} };

            bytes.push_back(static_cast<uint8_t>(value));
        }
        return { ErrorCode::OK, bytes };
    }

    //STRING / CHAR LITERAL
    if ((s.starts_with("\"") && s.ends_with("\"")) || (s.starts_with("'")  && s.ends_with("'"))) {
        std::string content = s.substr(1, s.size() - 2);

        if (content.size() != final_size)
            return { ErrorCode::INVALID_ARG, {} };

        for (unsigned char c : content)
            bytes.push_back(static_cast<uint8_t>(c));

        return { ErrorCode::OK, bytes };
    }


    //BASE 10
    {
        auto [ec, value] = string_utils::better_stoi(s, nullptr, 10);
        if (ec != ErrorCode::OK)
            return { ec, {} };

        for (size_t i = 0; i < final_size; ++i) {
            bytes.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
        }
        return { ErrorCode::OK, bytes };
    }
}


#endif