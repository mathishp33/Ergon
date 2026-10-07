#ifndef ERGON_PARSER_H
#define ERGON_PARSER_H

#include <bit>

#include "error.h"
#include "utils.h"

#include <unordered_map>
#include <utility>
#include <variant>
#include <cmath>
#include <cstdint>


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
    { "trp", 11 },
    { "cmp", 12 },
    { "fp", 13 },
    { "sp", 14 },
    { "pc", 15 },

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

enum class TokenType { Number, Identifier, Op, LParen, RParen, End };

struct Token {
    TokenType type;
    std::string text; // for Identifier/Op, or raw number text
    bool isFloat = false; // '.' or 'f'

    Token(TokenType t, std::string s, bool iF = false) : type(t), text(std::move(s)), isFloat(iF) {}
};

inline std::string strip_spaces_outside_quotes(const std::string& str) {
    std::string out;
    char quote = 0;
    for (char c : str) {
        if (quote) {
            out += c;
            if (c == quote)
                quote = 0;
            continue;
        }
        if (c == '\'') {
            quote = c;
            out += c;
            continue;
        }
        if (c == ' ')
            continue;
        out += c;
    }
    return out;
}

inline std::pair<ErrorInfo, std::vector<Token>> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string s = strip_spaces_outside_quotes(str);

    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];

        if (std::isdigit((unsigned char)c) || c == '.') {
            size_t start = i;
            bool isFloat = false;

            if (c == '0' && i + 1 < s.size() && std::string("xXbBoO").find(s[i + 1]) != std::string::npos) {
                bool hex = (s[i + 1] == 'x' || s[i + 1] == 'X');
                i += 2;
                while (i < s.size() && (hex ? std::isxdigit((unsigned char)s[i]) : std::isdigit((unsigned char)s[i]))) i++;
            } else {
                while (i < s.size() && (std::isdigit((unsigned char)s[i]) || s[i] == '.')) {
                    if (s[i] == '.') isFloat = true;
                    i++;
                }
                if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) { //exponent (1e5, 1e-5)
                    size_t j = i + 1;
                    if (j < s.size() && (s[j] == '+' || s[j] == '-')) j++;
                    if (j < s.size() && std::isdigit((unsigned char)s[j])) {
                        isFloat = true;
                        i = j;
                        while (i < s.size() && std::isdigit((unsigned char)s[i])) i++;
                    }
                }
                if (i < s.size() && s[i] == 'f') { isFloat = true; i++; } // float (1.5f)
            }
            std::string f_str = s.substr(start, i - start);
            tokens.emplace_back(TokenType::Number, f_str, isFloat);
        }
        else if (c == '\'') { //char
            if (i + 2 >= s.size() || s[i + 2] != '\'')
                return { { ErrorCode::INVALID_TOKEN, "invalid character literal" }, { } };
            tokens.emplace_back(TokenType::Number, std::to_string((int)s[i + 1]));
            i += 3;
        }
        else if (std::isalpha((unsigned char)c) || c == '_') { //variable or constant
            size_t start = i;
            while (i < s.size() && (std::isalnum((unsigned char)s[i]) || s[i] == '_')) i++;
            tokens.emplace_back(TokenType::Identifier, s.substr(start, i - start));
        }
        else if (c == '(') {
            tokens.emplace_back(TokenType::LParen, "(");
            i++;
        }
        else if (c == ')') {
            tokens.emplace_back(TokenType::RParen, ")");
            i++;
        }
        else { //operation
            static const char* ops[] = { "**", "<<", ">>", "<=", ">=", "==" };
            std::string op(1, c);
            for (auto o : ops)
                if (s.compare(i, 2, o) == 0) { op = o; break; }
            i += op.size();
            tokens.emplace_back(TokenType::Op, op);
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
    return Value{ static_cast<int32_t>(static_cast<uint32_t>(asInt(a)) + static_cast<uint32_t>(asInt(b))) };
}

inline Value sub(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) - asFloat(b) };
    return Value{ static_cast<int32_t>(static_cast<uint32_t>(asInt(a)) - static_cast<uint32_t>(asInt(b))) };
}

inline Value mul(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) * asFloat(b) };
    return Value{ static_cast<int32_t>(static_cast<uint32_t>(asInt(a)) * static_cast<uint32_t>(asInt(b))) };
}

inline Value div(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ asFloat(a) / asFloat(b) };
    const int32_t x = asInt(a), y = asInt(b);
    if (y == -1) return Value{ static_cast<int32_t>(0u - static_cast<uint32_t>(x)) };
    return Value{ x / y };
}

inline Value mod(const Value& a, const Value& b) {
    if (isFloat(a) || isFloat(b))
        return Value{ std::fmod(asFloat(a), asFloat(b)) };
    const int32_t x = asInt(a), y = asInt(b);
    if (y == -1) return Value{ int32_t{0} };
    return Value{ x % y };
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
                if (e.code != ErrorCode::OK) e_info = e;
                return { res };
            }
            auto [e, res] = string_utils::better_stoi(t.text);
            if (e.code != ErrorCode::OK) e_info = e;
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
        Value base = parsePrimary();
        if (matchOp("**")) {
            Value exponent = parseUnary();
            if (isFloat(base) || isFloat(exponent) || asInt(exponent) < 0)
                return Value{ std::pow(asFloat(base), asFloat(exponent)) };
            return Value{ static_cast<int32_t>(std::pow((double)asInt(base), (double)asInt(exponent))) };
        }
        return base;
    }

    Value parseUnary() {
        if (e_info.code != ErrorCode::OK) return 0;
        if (matchOp("-")) { Value v = parseUnary();
            return isFloat(v) ? Value{ -asFloat(v) } : Value{ -asInt(v) };
        }
        if (matchOp("~")) {
            Value v = parseUnary();
            return Value{ ~asInt(v) };
        }
        if (matchOp("+")) return parseUnary();
        return parsePower();
    }

    Value parseMulDivMod() {
        if (e_info.code != ErrorCode::OK) return 0;
        Value left = parseUnary();
        while (true) {
            if (matchOp("*")) {
                Value r = parseUnary();
                left = mul(left, r);
            }
            else if (matchOp("/")) {
                Value r = parseUnary();
                if ((isFloat(r) && asFloat(r) == 0.0f) || (!isFloat(r) && asInt(r) == 0)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation division by zero");
                    return 0;
                }
                left = div(left, r);
            }
            else if (matchOp("%")) {
                Value r = parseUnary();
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
                left = Value{ static_cast<int32_t>(static_cast<uint32_t>(asInt(left)) << (asInt(r) & 31)) };
            }
            else if (matchOp(">>")) {
                Value r = parseAddSub();
                if (isFloat(r) || isFloat(left)) {
                    e_info = ErrorInfo(ErrorCode::INVALID_OPERATION, "invalid operation \">>\"");
                    return 0;
                }
                left = Value{ asInt(left) >> (asInt(r) & 31) };
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


#endif