#ifndef ERGON_PARSER_H
#define ERGON_PARSER_H

#include "error.h"
#include "utils.h"

#include <unordered_map>
#include <utility>
#include <variant>


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
    { "zero", 15 },

    { "f0", 0 },
    { "f1", 1 },
    { "f2", 2 },
    { "f3", 3 },
    { "f4", 4 },
    { "f5", 5 },
    { "f6", 6 },
    { "f7", 7 },
    { "f8", 8 },
    { "f9", 9 },
    { "f10", 10 },
    { "f11", 11 },
    { "f12", 12 },
    { "f13", 13 },
    { "f14", 14 },
    { "f15", 15 }
};

static std::pair<ErrorInfo, uint8_t> parse_reg(const std::string& s) {
    if (reg_table.contains(s))
        return { { }, reg_table[s] };
    return { { ErrorCode::INVALID_REG, "invalid register \"" + s + "\"" }, 0 };
}

//temporary IMM parser

/*
Operations:
 +
 -
 *
 /
 %
 <<
 >>
 <
 >
 <=
 >=
 ==
 **
 &
 |
 ^
 ~
 ()
Types:
 HEXA->INT32 (0x... | numbers)
 BIN->INT32 (0b... | numbers)
 OCT->INT32 (0o... | numbers)
 (U)INT->INT32 (... | numbers)
 FLOAT->INT32 (.... | ...e... | numbers)
 CHAR->INT32 ('...' | letter)
 VAR/CST (... | letters)

*/

// inline std::pair<ErrorInfo, int> convert_to_bytes(std::string s, std::unordered_map<std::string, int32_t>& constants) {
//     if (constants.contains(s))
//         return { { }, constants[s] };
//     if (s.find('.'))
//         return string_utils::better_stof(s.substr(0, s.size() - s.back() == 'f'));
//     if (string_utils::rep_counter(s, '\'') == 2) {
//         if (s.size() != 3)
//             return { { ErrorCode::INVALID_IMM, "invalid" + s.substr(2, s.size() - 3) + "character" }, 0 };
//         return { { }, s[1] };
//     }
//     return string_utils::better_stoi(s);
// }

enum class TokenType { Number, Identifier, Op, LParen, RParen, End };

struct Token {
    TokenType type;
    std::string text; // for Identifier/Op, or raw number text
    bool isFloat = false; // '.' or 'f'

    Token(TokenType t, std::string s, bool iF = false) : type(t), text(std::move(s)), isFloat(iF) {}
};

inline std::pair<ErrorInfo, std::vector<Token>> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string s = string_utils::remove_char(str, ' ');

    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];

        if (std::isdigit(c) || c == '.') { //number: int or float
            size_t start = i;
            if (c == '0' && i + 1 < s.size())
                if (s[i + 1] == 'x' || s[i + 1] == 'X' || s[i + 1] == 'b' || s[i + 1] == 'B')
                    i++;
            while (i < s.size() && (std::isdigit(s[i]) || s[i] == '.' || s[i] == 'f')) i++;
            std::string sub_str = s.substr(start, i - start);
            tokens.emplace_back(TokenType::Number, sub_str, sub_str.find('f') != std::string::npos || sub_str.find('.') != std::string::npos);
        }
        else if (c == '\'') { //char
            size_t start = i++;
            while (i < s.size() && s[i] != '\'') i++;
            i++;
            if (i - start != 3) return { { ErrorCode::INVALID_TOKEN, "invalid token \"" + s.substr(start, i - start) + "\"" }, { } };
            tokens.emplace_back(TokenType::Number, s.substr(start, i - start));
        }
        else if (std::isalpha(c) || c == '_') { //variable or constant
            size_t start = i;
            while (i < s.size() && (std::isalnum(s[i]) || s[i] == '_')) i++;
            tokens.emplace_back(TokenType::Identifier, s.substr(start, i - start));
        }
        else if (c == '(') {
            tokens.emplace_back(TokenType::LParen, std::to_string(c));
            i++;
        }
        else if (c == ')') {
            tokens.emplace_back(TokenType::RParen, std::to_string(c));
            i++;
        }
        else { //operation
            if (s[i] == '-') {
                tokens.emplace_back(TokenType::Op, "-");
                i++;
            }
            else {
                size_t start = i;
                while (i < s.size() && !std::isdigit(s[i]) && !std::isalpha(s[i]) && s[i] != '_' && s[i] != '(' && s[i] != ')' && s[i] != ' ' && s[i] != '-')
                    i++;
                tokens.emplace_back(TokenType::Op, s.substr(start, i - start));
            }
        }
    }
    tokens.emplace_back(TokenType::End, "");
    return { { }, tokens };
}

using Value = std::variant<int32_t, float>;

inline bool isFloat(const Value& v) {
    return std::holds_alternative<float>(v);
}

inline float asFloat(const Value& v) {
    return isFloat(v) ? std::get<float>(v) : static_cast<float>(std::get<int32_t>(v));
}

inline int32_t asInt(const Value& v) {
    if (isFloat(v))
        return (int32_t)asFloat(v);
    return std::get<int32_t>(v);
}

inline Value add(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) + asFloat(b) };
    return Value{ asInt(a) + asInt(b) };
}

inline Value sub(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) - asFloat(b) };
    return Value{ asInt(a) - asInt(b) };
}

inline Value mul(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) * asFloat(b) };
    return Value{ asInt(a) * asInt(b) };
}

inline Value div(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) / asFloat(b) };
    return Value{ asInt(a) / asInt(b) };
}

inline Value mod(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ std::fmod(asFloat(a), asFloat(b)) };
    return Value{ asInt(a) % asInt(b) };
}

inline Value greater(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) > asFloat(b) };
    return Value{ asInt(a) > asInt(b) };
}

inline Value lesser(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) < asFloat(b) };
    return Value{ asInt(a) < asInt(b) };
}

inline Value greaterOrEqual(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) >= asFloat(b) };
    return Value{ asInt(a) >= asInt(b) };
}

inline Value lesserOrEqual(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) <= asFloat(b) };
    return Value{ asInt(a) <= asInt(b) };
}

inline Value equal(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) == asFloat(b) };
    return Value{ asInt(a) == asInt(b) };
}


class Parser {
public:
    std::unordered_map<std::string, Value> values;
    ErrorInfo e_info;

    Parser(std::vector<Token> toks, std::unordered_map<std::string, Value> vals) : tokens(std::move(toks)), values(std::move(vals)) {}

    Value parse() {
        Value result = parseBitwiseOr();
        if (e_info.code != ErrorCode::OK) return result;
        expect(TokenType::End); // safeguard
        return result;
    }

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& peek() const {
        return tokens[pos];
    }
    Token advance() {
        return tokens[pos++];
    }

    bool matchOp(const std::string& op) {
        if (peek().type == TokenType::Op && peek().text == op) { advance(); return true; }
        return false;
    }
    void expect(TokenType t) {
        if (peek().type != t)
            e_info = ErrorInfo(ErrorCode::INVALID_TOKEN, "invalid token \"" + peek().text + "\"");
    }

    Value parsePrimary() {
        const Token& t = peek();

        if (t.type == TokenType::Number) {
            advance();
            if (t.isFloat) {
                auto [e, res] = string_utils::better_stof(t.text);
                e_info = e;
                return { res };
            }
            auto [e, res] = string_utils::better_stoi(t.text);
            e_info = e;
            return { res };
        }
        if (t.type == TokenType::Identifier) {
            advance();
            auto it = values.find(t.text);
            if (it == values.end()) {
                e_info = ErrorInfo(ErrorCode::UNKNOWN_VARIABLE, "unknown variable \"" + t.text + "\"");
                return 0;
            }
            return it->second;
        }
        if (t.type == TokenType::LParen) {
            advance();
            Value v = parseBitwiseOr();
            expect(TokenType::RParen);
            if (e_info.code != ErrorCode::OK ) return 0;
            advance();
            return v;
        }
        e_info = ErrorInfo(ErrorCode::INVALID_TOKEN, "invalid token (unknown type) \"" + peek().text + "\"");
        return 0;
    }

    Value parsePower() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value base = parseUnary();
        if (matchOp("**")) {
            Value exponent = parsePower();
            if (isFloat(base) || isFloat(exponent) || asFloat(exponent) < 0)
                return Value{ std::pow(asFloat(base), asFloat(exponent)) };
            else
                return Value{ static_cast<int32_t>(std::pow(asFloat(base), asFloat(exponent))) };
        }
        return base;
    }

    Value parseUnary() {
        if (e_info.code != ErrorCode::OK) return 0;
        if (matchOp("-")) {
            Value v = parseUnary();
            return isFloat(v) ? Value{ -asFloat(v) } : Value{ -asInt(v) };
        }
        if (matchOp("~")) {
            Value v = parseUnary();
            return Value{ ~asInt(v) };
        }
        return parsePrimary();
    }

    Value parseMulDivMod() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parsePower();
        while (true) {
            if (matchOp("*")) {
                Value r = parsePower();
                left = mul(left, r);
            }
            else if (matchOp("/")) {
                Value r = parsePower();
                if ((isFloat(r) && asFloat(r) == 0.0f) || (!isFloat(r) && asInt(r) == 0)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation division by zero");
                    return 0;
                }
                left = div(left, r);
            }
            else if (matchOp("%")) {
                Value r = parsePower();
                if ((isFloat(r) && asFloat(r) == 0.0f) || (!isFloat(r) && asInt(r) == 0)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation mod by zero");
                    return 0;
                }
                left = mod(left, r);
            }
            else break;
        }
        return left;
    }

    Value parseAddSub() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseMulDivMod();
        while (true) {
            if (matchOp("+")) {
                Value r = parseMulDivMod();
                left = add(left, r);
            }
            else if (matchOp("-")) {
                Value r = parseMulDivMod();
                left = sub(left, r);
            }
            else break;
        }
        return left;
    }

    Value parseShift() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseAddSub();
        while (true) {
            if (matchOp("<<")) {
                Value r = parseAddSub();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \"<<\"");
                    return 0;
                }
                left = asInt(left) << asInt(r);
            }
            else if (matchOp(">>")) {
                Value r = parseAddSub();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \">>\"");
                    return 0;
                }
                left = asInt(left) >> asInt(r);
            }
            else break;
        }
        return left;
    }

    Value parseRelational() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseShift();
        while (true) {
            if (matchOp(">")) {
                Value r = parseShift();
                left = greater(left, r);
            }
            else if (matchOp("<")) {
                Value r = parseShift();
                left = lesser(left, r);
            }
            else if (matchOp("<=")) {
                Value r = parseShift();
                left = lesserOrEqual(left, r);
            }
            else if (matchOp(">=")) {
                Value r = parseShift();
                left = greaterOrEqual(left, r);
            }
            else break;
        }
        return left;
    }

    Value parseEquality() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseRelational();
        while (true) {
            if (matchOp("==")) {
                Value r = parseRelational();
                left = equal(left, r);
            }
            else break;
        }
        return left;
    }

    Value parseBitwiseAnd() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseEquality();
        while (true) {
            if (matchOp("&")) {
                Value r = parseEquality();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \"&\"");
                    return 0;
                }
                left = asInt(left) & asInt(r);
            }
            else break;
        }
        return left;
    }

    Value parseBitwiseXor() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseBitwiseAnd();
        while (true) {
            if (matchOp("^")) {
                Value r = parseBitwiseAnd();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \"^\"");
                    return 0;
                }
                left = asInt(left) ^ asInt(r);
            }
            else break;
        }
        return left;
    }

    Value parseBitwiseOr() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseBitwiseXor();
        while (true) {
            if (matchOp("|")) {
                Value r = parseBitwiseXor();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \"|\"");
                    return 0;
                }
                left = asInt(left) | asInt(r);
            }
            else break;
        }
        return left;
    }
};


inline std::pair<ErrorInfo, int32_t> parse_expr(const std::string& expr, const std::unordered_map<std::string, Value>& csts, const std::unordered_map<std::string, Value>& vars) {
    if (expr.empty()) return { { }, 0 };
    auto [e, tokens] = tokenize(expr);
    if (e.code != ErrorCode::OK) return { e, 0 };

    std::unordered_map<std::string, Value> merged = csts;
    merged.insert(vars.begin(), vars.end());

    auto parser = Parser(tokens, merged);

    Value val = parser.parse();
    if (parser.e_info.code != ErrorCode::OK) return { parser.e_info, 0 };

    return { { }, isFloat(val) ? std::bit_cast<int32_t>(asFloat(val)) : asInt(val) };
}

inline std::pair<ErrorInfo, std::pair<std::string, int32_t>> parse_var(const std::string& var, std::unordered_map<std::string, Value>& csts, const std::unordered_map<std::string, Value>& vars) {
    std::string name;

    //var[index]
    auto lb = var.find('[');
    if (lb != std::string::npos) {
        auto rb = var.find(']');
        name = var.substr(0, lb);

        std::string inside = var.substr(lb + 1, rb - lb - 1);

        auto [e, val] = parse_expr(inside, csts, vars);
        if (e.code != ErrorCode::OK) return { e, { "", 0 } };

        return { { }, { name, val } };
    }

    name = var;
    return { { }, { name, 0 } };
}


#endif