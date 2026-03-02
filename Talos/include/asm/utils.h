#ifndef ERGON_UTILS_H
#define ERGON_UTILS_H

#include "error.h"

#include <vector>
#include <string>
#include <cctype>
#include <limits>

namespace string_utils {

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
        if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
        return -1;
    }

    inline std::string lower_case(const std::string& str) {
        std::string result = str;
        for (char& c : result)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return result;
    }

    inline std::string replace_null(std::string s, char a) {
        size_t move = 0;
        size_t i = 0;

        for (; i < s.size(); ++i) {
            if (s[i] == a) {
                move++;
            } else {
                s[i - move] = s[i];
            }
        }

        s.resize(s.size() - move);
        return s;
    }

    // returns { error_code, result }
    inline std::pair<ErrorInfo, int> better_stoi(const std::string& str, size_t* idx = nullptr, int base = 10) {
        if (base < 2 || base > 36)
            return { { ErrorCode::INVALID_BASE, "invalid base in stoi (base should be between 2 and 36)" }, 0};

        size_t i = 0;
        int sign = 1;

        const std::string cleaned_str = replace_null(str, '_');
        if (cleaned_str.empty()) return { { }, 0 };

        if (cleaned_str[i] == '+' || cleaned_str[i] == '-') {
            if (cleaned_str[i] == '-') sign = -1;
            ++i;
        }

        int result = 0;
        size_t start = i;

        for (; i < cleaned_str.size(); ++i) {
            int digit = char_to_int(cleaned_str[i]);
            if (digit < 0 || digit >= base)
                break;

            if (sign == 1 && result > (INT_MAX - digit) / base)
                return { { ErrorCode::STOI_POS_OVERFLOW, "positive overflow in stoi"}, 0};
            if (sign == -1 && result > (INT_MAX - digit + 1) / base)
                return { { ErrorCode::STOI_NEG_OVERFLOW, "negative overflow in stoi" }, 0};

            result = result * base + digit;
        }

        if (i == start)
            return { { ErrorCode::STOI_INVALID_CHAR, "invalid char in stoi" }, 0};

        if (idx)
            *idx = i;

        return {{ ErrorCode::OK, ""}, result * sign};
    }

    inline std::string remove_char(const std::string& s, char c) {
        std::string result;
        result.reserve(s.size());

        for (char ch : s)
            if (ch != c)
                result.push_back(ch);

        return result;
    }

    inline std::string trim_spaces(const std::string& str) {
        auto l = str.find_first_not_of(" \t");
        auto r = str.find_last_not_of(" \t");
        if (l == std::string::npos) return "";
        return str.substr(l, r - l + 1);
    }

    static std::string normalize(std::string line) {
        //comments
        line = line.substr(0, line.find(';'));

        return trim_spaces(line);
    }
}


#endif