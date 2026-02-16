#ifndef ERGON_UTILS_H
#define ERGON_UTILS_H

#include "error.h"

#include <vector>
#include <string>
#include <cctype>
#include <limits>

inline std::vector<std::string> slice_str(const std::string& str, char wanted_char) {
    std::vector<std::string> result;
    std::string buffer;

    for (char c : str) {
        if (c == wanted_char) {
            result.push_back(buffer);
            buffer.clear();
        } else
            buffer += c;
    }
    result.push_back(buffer);
    return result;
}

inline unsigned int rep_counter(const std::string& str, char wanted_char) {
    unsigned int i = 0;
    for (char c : str) {
        if (c == wanted_char) i++;
    }
    return i;
}

inline int char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return 0;
}

inline std::string lower_case(const std::string& str) {
    std::string result = str;
    for (char& c : result)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

// returns { error_code, result }
inline std::pair<ErrorCode, int> better_stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
    if (base < 2 || base > 36)
        return {ErrorCode::STOI_OVERFLOW, 0};

    size_t i = 0;
    int sign = 1;

    if (str[i] == '+' || str[i] == '-') {
        if (str[i] == '-') sign = -1;
        ++i;
    }

    int result = 0;
    size_t start = i;

    for (; i < str.size(); ++i) {
        int digit = char_to_int(str[i]);
        if (digit < 0 || digit >= base)
            break;

        if (result > (std::numeric_limits<int>::max() - digit) / base)
            return {ErrorCode::STOI_OVERFLOW, 0};

        result = result * base + digit;
    }

    if (i == start)
        return {ErrorCode::INVALID_CHAR, 0};

    if (idx)
        *idx = i;

    return {ErrorCode::OK, result * sign};
}

#endif