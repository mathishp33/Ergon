#ifndef ERGON_UTILS_H
#define ERGON_UTILS_H

#include "error.h"

#include <vector>
#include <string>
#include <cctype>
#include <charconv>

namespace string_utils {

    inline std::string trim_spaces(const std::string& str) {
        auto l = str.find_first_not_of(" \t");
        auto r = str.find_last_not_of(" \t");
        if (l == std::string::npos) return "";
        return str.substr(l, r - l + 1);
    }

    inline std::vector<std::string> slice_str(const std::string& str, char wanted_char) {
        std::vector<std::string> result;
        std::string buffer;

        for (char c : str) {
            if (c == wanted_char) {
                result.push_back(buffer);
                buffer.clear();
            }
            else
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

    inline void replace_string(std::string& s, const std::string& from, const std::string& to) {
        if (from.empty() || from == to)
            return;

        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.length(), to);
            pos += to.length();
        }
    }

    inline void replace_string_as_token(std::string& s, const std::string& from, const std::string& to) {
        if (from.empty() || from == to)
            return;

        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            size_t dec = pos - 1;
            size_t inc = pos + from.size();
            if ((pos == 0 || s[dec] == ',' || s[dec] == ' ' || s[dec] == '\n' || s[dec] == '{') && (inc >= s.size() || s[inc] == ',' || s[inc] == ' ' || s[inc] == '\n' || s[dec] == '}')) {
                s.replace(pos, from.length(), to);
                pos += to.length();
            }
            else {
                pos += from.length();
            }
        }
    }

    inline std::pair<ErrorInfo, int32_t> better_stoi(const std::string& str) {
        if (str.starts_with("\'") && str.ends_with("\'")) {
            if (str.size() != 3) return { { ErrorCode::STOI_INVALID_CHAR, "invalid char in stoi" }, 0 };
            std::string sub_str = str.substr(1, str.size() - 2);
            return { { }, (int)sub_str[0] };
        }

        int base = 10;
        if (str.length() > 2) {
            if (str.starts_with("0x"))
                base = 16;
            if (str.starts_with("0b"))
                base = 2;
            if (str.starts_with("0o"))
                base = 8;
        }
        std::string cleaned_str = str;
        if (base != 10)
            cleaned_str = str.substr(2);

        int32_t result = 0;
        auto r = std::from_chars(cleaned_str.data(), cleaned_str.data() + cleaned_str.size(), result, base);

        if (r.ec == std::errc::invalid_argument)
            return { { ErrorCode::STOI_INVALID_CHAR, "invalid char in stoi" }, 0 };
        if (r.ec == std::errc::result_out_of_range)
            return { { ErrorCode::STOI_OVERFLOW, "overflow in stoi" }, 0 };
        return { { }, result };
    }

    inline std::pair<ErrorInfo, float> better_stof(const std::string& str) {
        float result = 0;
        auto r = std::from_chars(str.data(), str.data() + str.size(), result); // false error

        if (r.ec == std::errc::invalid_argument)
            return { { ErrorCode::STOF_INVALID_CHAR, "invalid char in stof" }, 0.0f };
        if (r.ec == std::errc::result_out_of_range)
            return { { ErrorCode::STOF_OVERFLOW, "overflow in stof" }, 0.0f };
        return { { }, result };
    }

    inline std::string remove_char(const std::string& s, char c) {
        std::string result;
        result.reserve(s.size());

        for (char ch : s)
            if (ch != c)
                result.push_back(ch);

        return result;
    }

    static std::string normalize(std::string line) {
        //comments
        line = line.substr(0, line.find(';'));

        return trim_spaces(line);
    }

    inline void remove_blank_lines(std::string& s) {
        std::string result;

        size_t begin = 0;
        while (begin < s.size()) {
            size_t end = s.find('\n', begin);
            if (end == std::string::npos)
                end = s.size();

            bool blank = true;
            for (size_t i = begin; i < end; ++i) {
                if (s[i] != ' ' && s[i] != '\t') {
                    blank = false;
                    break;
                }
            }

            if (!blank) {
                result.append(s, begin, end - begin);

                if (end != s.size())
                    result += '\n';
            }
            begin = end + 1;
        }
        s = std::move(result);
    }

    inline bool check_cst_name(const std::string& s) {
        if (s.empty())
            return false;
        for (char c : s)
            if (!(std::isdigit(c) || std::isalpha(c) || c == '_'))
                return false;
        if (std::isdigit(s.front()))
            return false;
        return true;
    }
}

//true if not good
inline bool var_is_constant(const std::string& instr) {
    if (instr == "stb") return true;
    if (instr == "sth") return true;
    if (instr == "stw") return true;
    if (instr == "fstw") return true;
    return false;
}


#endif