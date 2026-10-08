#ifndef ERGON_PREPROCESSOR_H
#define ERGON_PREPROCESSOR_H
#include "../error.h"

#include <cctype>
#include <deque>
#include <functional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct RecursionHandler {
    size_t max_depth = 1000;
    size_t max_lines = 100000000;

    [[nodiscard]] bool depth_exceeded(size_t depth) const {
        return depth > max_depth;
    }
};

struct PreProcesser {
    std::unordered_map<std::string, Value> constants; // %equ
    std::unordered_map<std::string, Value> variables; // %assign
    std::unordered_map<std::string, Define> defines; // %define
    std::unordered_map<std::string, Macro> macros; // %macro
    const std::vector<std::pair<std::string, std::string>>* includes = nullptr;

    struct SourceLoc { uint32_t file; uint32_t line; };
    std::vector<std::string> file_names;
    std::vector<SourceLoc> line_map;
    size_t last_file = 0;

    size_t file_id(const std::string& name) {
        for (size_t i = 0; i < file_names.size(); i++)
            if (file_names[i] == name) return i;
        file_names.push_back(name);
        return file_names.size() - 1;
    }

    static std::vector<std::string> split_lines(const std::string& s) {
        std::vector<std::string> out;
        size_t start = 0;
        while (true) {
            const size_t nl = s.find('\n', start);
            std::string l = s.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
            if (!l.empty() && l.back() == '\r') l.pop_back();
            out.push_back(std::move(l));
            if (nl == std::string::npos) break;
            start = nl + 1;
        }
        return out;
    }

    static std::string delimit_string(const std::string& s, size_t& idx, const std::function<bool(char, size_t)>& condition) {
        size_t start = idx;
        for (; idx < s.size() && condition(s[idx], idx); idx++) {}
        return s.substr(start, idx - start);
    }

    static std::string leading_identifier(const std::string& s) {
        size_t idx = 0;
        return delimit_string(s, idx, [](char c, size_t) {
            return std::isalnum((unsigned char)c) || c == '_';
        });
    }

    static bool is_ident_start(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }
    static bool is_ident_char(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    static bool is_if_open(const std::string& w) {
        return w == "%if" || w == "%ifn" || w == "%ifdef" || w == "%ifndef";
    }
    static bool is_if_close(const std::string& w) {
        return w == "%endif" || w == "%endifdef";
    }

    static bool is_expr_word(const std::string& w) {
        return w == "%if" || w == "%ifn" || w == "%elseif" || w == "%elseifn";
    }

    static bool is_def_word(const std::string& w) {
        return w == "%ifdef" || w == "%ifndef" || w == "%elseifdef" || w == "%elseifndef";
    }
    static bool is_negated_word(const std::string& w) {
        return w == "%ifn" || w == "%ifndef" || w == "%elseifn" || w == "%elseifndef";
    }

    static bool is_else_word(const std::string& w) {
        return w == "%else" || w == "%elseif" || w == "%elseifn" || w == "%elseifdef" || w == "%elseifndef" || w == "%elsedef";
    }

    static bool is_identifier(const std::string& s) {
        if (s.empty() || !is_ident_start(s[0]))
            return false;
        for (char c : s)
            if (!is_ident_char(c))
                return false;
        return true;
    }

    static std::string trim(const std::string& s) {
        size_t b = 0, e = s.size();
        while (b < e && std::isspace(static_cast<unsigned char>(s[b])))
            b++;
        while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1])))
            e--;
        return s.substr(b, e - b);
    }

    static std::string first_token(const std::string& s) {
        size_t i = 0;
        while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i])))
            i++;
        return s.substr(0, i);
    }

    static bool parse_call_args(const std::string& s, size_t& pos, std::vector<std::string>& args) {
        args.clear();
        size_t depth = 1;
        std::string cur;
        char quote = 0;
        for (size_t i = pos + 1; i < s.size(); i++) {
            char c = s[i];
            if (quote) {
                cur += c;
                if (c == quote) quote = 0;
                continue;
            }
            if (c == '"' || c == '\'') { quote = c; cur += c; continue; }
            if (c == '(') depth++;
            else if (c == ')') {
                if (--depth == 0) {
                    args.push_back(trim(cur));
                    if (args.size() == 1 && args[0].empty()) args.clear(); //no args
                    pos = i + 1;
                    return true;
                }
            }
            else if (c == ',' && depth == 1) {
                args.push_back(trim(cur));
                cur.clear();
                continue;
            }
            cur += c;
        }
        return false;
    }

    static std::string substitute_params(const std::string& text, const std::vector<std::string>& params,
        const std::vector<std::string>& args) {
        std::string out;
        size_t p = 0;
        while (p < text.size()) {
            char c = text[p];
            if (c == '"' || c == '\'') {
                size_t q = text.find(c, p + 1);
                q = (q == std::string::npos) ? text.size() : q + 1;
                out += text.substr(p, q - p);
                p = q;
                continue;
            }
            if (is_ident_char(c)) {
                size_t q = p;
                while (q < text.size() && is_ident_char(text[q])) q++;
                std::string id = text.substr(p, q - p);
                bool replaced = false;
                if (is_ident_start(c)) {
                    for (size_t j = 0; j < params.size() && j < args.size(); j++) {
                        if (params[j] == id) {
                            out += args[j]; replaced = true;
                            break;
                        }
                    }
                }
                if (!replaced) out += id;
                p = q;
                continue;
            }
            out += c;
            p++;
        }
        return out;
    }

    bool is_defined(const std::string& name) const {
        return defines.contains(name) || constants.contains(name) || variables.contains(name) || macros.contains(name);
    }

    ErrorInfo expand_defines(std::string& line, size_t src, const RecursionHandler& RH) const {
        if (defines.empty()) return { };
        for (size_t pass = 0; ; pass++) {
            if (pass > RH.max_depth || line.size() > (1u << 22))
                return { ErrorCode::PREPROC_RECURSION, "infinite recursion in define expansion", src };
            bool changed = false;
            std::string out;
            size_t p = 0;
            while (p < line.size()) {
                char c = line[p];
                if (c == '"' || c == '\'') {
                    size_t q = line.find(c, p + 1);
                    q = (q == std::string::npos) ? line.size() : q + 1;
                    out += line.substr(p, q - p);
                    p = q;
                    continue;
                }
                if (is_ident_char(c)) {
                    size_t q = p;
                    while (q < line.size() && is_ident_char(line[q])) q++;
                    std::string id = line.substr(p, q - p);
                    auto it = is_ident_start(c) ? defines.find(id) : defines.end();
                    if (it == defines.end()) {
                        out += id; p = q;
                        continue;
                    }

                    const Define& d = it->second;
                    if (d.parameters.empty()) {
                        out += d.replacement;
                        p = q;
                        changed = true;
                    }
                    else if (q < line.size() && line[q] == '(') {
                        size_t pos = q;
                        std::vector<std::string> args;
                        if (!parse_call_args(line, pos, args))
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", src };
                        if (args.size() != d.parameters.size())
                            return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " +
                                std::to_string(d.parameters.size()), src };
                        out += substitute_params(d.replacement, d.parameters, args);
                        p = pos;
                        changed = true;
                    }
                    else {
                        out += id;
                        p = q;
                    }
                    continue;
                }
                out += c;
                p++;
            }
            line = out;
            if (!changed) return { };
        }
    }

    static ErrorInfo parse_signature(const std::string& rest, size_t src, std::string& name, std::vector<std::string>& params,
        std::string& tail, bool& has_params) {
        name = leading_identifier(rest);
        if (name.empty() || !string_utils::check_cst_name(name))
            return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", src };
        const std::string after = rest.substr(name.size());
        params.clear();
        has_params = false;
        if (!after.empty() && after[0] == '(') {
            size_t pos = 0;
            if (!parse_call_args(after, pos, params))
                return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", src };
            for (const auto& prm : params)
                if (!is_identifier(prm))
                    return { ErrorCode::INVALID_NAME, "invalid parameter name \"" + prm + "\"", src };
            has_params = true;
            tail = trim(after.substr(pos));
        }
        else if (after.empty() || std::isspace((unsigned char)after[0])) {
            tail = trim(after);
        }
        else {
            return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", src };
        }
        return { };
    }

    struct Line {
        std::string text;
        size_t src; //line nbr
        size_t depth; //macro depth
        std::string chain;
        size_t file = 0;
    };

    static bool take_block(std::deque<Line>& work, bool conditional, std::vector<Line>& body, std::string& closer) {
        size_t depth = 0;
        while (!work.empty()) {
            Line l = std::move(work.front());
            work.pop_front();
            std::string w = first_token(string_utils::normalize(l.text));
            bool open = conditional ? is_if_open(w) : (w == "%rep");
            bool close = conditional ? is_if_close(w) : (w == "%endrep");
            if (open) depth++;
            else if (close) {
                if (depth == 0) { closer = w; return true; }
                depth--;
            }
            body.push_back(std::move(l));
        }
        return false;
    }

    static void push_front_lines(std::deque<Line>& work, const std::vector<Line>& lines) {
        for (auto it = lines.rbegin(); it != lines.rend(); ++it)
            work.push_front(*it);
    }

    struct Branch {
        std::string word; // %if, %ifdef, %elseif, %elseifdef, %else, %elsedef
        std::string arg;
        size_t src;
        std::vector<Line> body;
    };


    ErrorInfo process(std::deque<Line>& work, std::vector<std::string>& out, const RecursionHandler& RH) {
        size_t processed = 0;
        while (!work.empty()) {
            Line cur = std::move(work.front());
            work.pop_front();
            last_file = cur.file;
            if (RH.depth_exceeded(cur.depth) || ++processed > RH.max_lines)
                return { ErrorCode::PREPROC_RECURSION, "infinite recursion in the preprocessor", cur.src };

            std::string norm = string_utils::normalize(cur.text);
            if (norm.empty()) continue;

            std::string word = norm[0] == '%' ? first_token(norm) : std::string();
            std::string rest = word.empty() ? std::string() : trim(norm.substr(word.size()));

            if (word == "%equ" || word == "%assign") {
                std::string name = leading_identifier(rest);
                if (name.empty() || !string_utils::check_cst_name(name))
                    return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", cur.src };
                std::string expr = trim(rest.substr(name.size()));
                if (ErrorInfo e = expand_defines(expr, cur.src, RH); e.code != ErrorCode::OK) return e;
                auto [e, value] = parse_expr(expr, constants, variables);
                if (e.code != ErrorCode::OK) { e.index_line = cur.src; return e; }

                if (word == "%equ") {
                    if (constants.contains(name))
                        return { ErrorCode::DUPLICATE_CONSTANT, "duplicate constant \"" + name + "\"", cur.src };
                    constants[name] = value;
                }
                else {
                    variables[name] = value;
                }
                continue;
            }

            if (word == "%define") {
                std::string name, tail;
                std::vector<std::string> params;
                bool has_params;
                if (ErrorInfo e = parse_signature(rest, cur.src, name, params, tail, has_params); e.code != ErrorCode::OK)
                    return e;
                if (defines.contains(name))
                    return { ErrorCode::DUPLICATE_DEFINE, "duplicate define \"" + name + "\"", cur.src };
                defines[name] = { params, tail };
                continue;
            }

            if (word == "%macro") {
                std::string name, tail;
                std::vector<std::string> params;
                bool has_params;
                if (ErrorInfo e = parse_signature(rest, cur.src, name, params, tail, has_params); e.code != ErrorCode::OK)
                    return e;

                std::vector<std::string> body;
                bool closed = false;
                while (!work.empty()) {
                    Line l = std::move(work.front());
                    work.pop_front();
                    if (first_token(string_utils::normalize(l.text)) == "%endmacro") { closed = true; break; }
                    body.push_back(l.text);
                }
                if (!closed)
                    return { ErrorCode::MISSING_ENDMACRO, "missing an %endmacro", cur.src };
                if (macros.contains(name))
                    return { ErrorCode::DUPLICATE_MACRO, "duplicate macro \"" + name + "\"", cur.src };
                macros[name] = { params, body };
                continue;
            }

            if (word == "%rep") {
                std::string expr = rest;
                if (ErrorInfo e = expand_defines(expr, cur.src, RH); e.code != ErrorCode::OK) return e;
                auto [e_0, value] = parse_expr(expr, constants, variables);
                if (e_0.code != ErrorCode::OK) {
                    e_0.index_line = cur.src;
                    return e_0;
                }

                std::vector<Line> body;
                if (std::string closer; !take_block(work, false, body, closer))
                    return { ErrorCode::MISSING_ENDREP, "missing an %endrep", cur.src };

                if (value > 0 && !body.empty() && static_cast<size_t>(value) > RH.max_lines / body.size())
                    return { ErrorCode::PREPROC_RECURSION, "%rep expansion is too large", cur.src };
                for (long long k = 0; k < value; k++)
                    push_front_lines(work, body);
                continue;
            }

            if (is_if_open(word)) {
                const bool is_ifdef = (word == "%ifdef" || word == "%ifndef");
                std::vector<Line> block;
                std::string closer;
                if (!take_block(work, true, block, closer) || closer != (is_ifdef ? "%endifdef" : "%endif")) {
                    if (is_ifdef) return { ErrorCode::MISSING_ENDIFDEF, "missing an %endifdef", cur.src };
                    return { ErrorCode::MISSING_ENDIF, "missing an %endif", cur.src };
                }

                std::vector<Branch> branches;
                branches.push_back({ word, rest, cur.src, { } });
                size_t depth = 0;
                bool seen_else = false;
                for (Line& l : block) {
                    std::string n = string_utils::normalize(l.text);
                    std::string w = first_token(n);
                    if (is_if_open(w))
                        depth++;
                    else if (is_if_close(w))
                        depth--;
                    else if (depth == 0 && is_else_word(w)) {
                        if (seen_else)
                            return { ErrorCode::ELSE_AFTER_ELSE, "unexpected \"" + w + "\" after %else", l.src };
                        if (w == "%else" || w == "%elsedef")
                            seen_else = true;
                        branches.push_back({ w, trim(n.substr(w.size())), l.src, { } });
                        continue;
                    }
                    branches.back().body.push_back(std::move(l));
                }

                for (const Branch& b : branches) {
                    bool take = false;
                    if (is_expr_word(b.word)) {
                        std::string expr = b.arg;
                        if (ErrorInfo e = expand_defines(expr, b.src, RH); e.code != ErrorCode::OK)
                            return e;
                        auto [e, value] = parse_expr(expr, constants, variables);
                        if (e.code != ErrorCode::OK) { e.index_line = b.src; return e; }
                        take = (value != 0);
                    }
                    else if (is_def_word(b.word)) {
                        if (!string_utils::check_cst_name(b.arg))
                            return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", b.src };
                        take = is_defined(b.arg);
                    }
                    else {
                        take = true; //%else || %elsedef
                    }
                    if (is_negated_word(b.word)) take = !take;

                    if (take) {
                        push_front_lines(work, b.body);
                        break;
                    }
                }
                continue;
            }

            if (word == "%include") {
                if (rest.size() < 3 || rest.front() != '"' || rest.back() != '"' || rest.find('"', 1) != rest.size() - 1)
                    return { ErrorCode::INVALID_INCLUDE, "expected %include \"name\"", cur.src };

                const std::string file_name = rest.substr(1, rest.size() - 2);
                const std::string* content = nullptr;
                if (includes)
                    for (const auto& [name, text] : *includes)
                        if (name == file_name) { content = &text; break; }
                if (!content)
                    return { ErrorCode::INCLUDE_NOT_FOUND, "cannot find include \"" + file_name + "\"", cur.src };

                if (cur.chain.find("|" + file_name + "|") != std::string::npos)
                    return { ErrorCode::INCLUDE_CYCLE, "include cycle on \"" + file_name + "\"", cur.src };

                const size_t fid = file_id(file_name);
                const std::string chain = cur.chain + file_name + "|";
                const std::vector<std::string> inc_lines = split_lines(*content);
                std::vector<Line> body;
                for (size_t k = 0; k < inc_lines.size(); k++)
                    body.push_back({ inc_lines[k], k, cur.depth + 1, chain, fid });
                push_front_lines(work, body);
                continue;
            }
            if (word == "%endif" || word == "%endifdef" || word == "%endrep" || word == "%endmacro" || is_else_word(word))
                return { ErrorCode::UNEXPECTED_DIRECTIVE, "unexpected \"" + word + "\" without a matching opening directive",
                    cur.src };

            std::string line = cur.text;
            if (ErrorInfo e = expand_defines(line, cur.src, RH); e.code != ErrorCode::OK) return e;
            std::string n = string_utils::normalize(line);
            if (n.empty()) continue;

            std::string id = leading_identifier(n);
            auto mit = id.empty() ? macros.end() : macros.find(id);
            if (mit != macros.end()) {
                const Macro& macro = mit->second;
                std::vector<std::string> args;
                bool is_call = false;
                if (n.size() > id.size() && n[id.size()] == '(') {
                    size_t pos = id.size();
                    if (!parse_call_args(n, pos, args))
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", cur.src };
                    if (!trim(n.substr(pos)).empty())
                        return { ErrorCode::MISMATCHED_PAR, "unexpected text after macro call", cur.src };
                    is_call = true;
                }
                else if (n == id && macro.parameters.empty()) {
                    is_call = true;
                }
                else if (!macro.parameters.empty()) {
                    return { ErrorCode::MISMATCHED_PAR, "expected '(' after macro name \"" + id + "\"", cur.src };
                }

                if (is_call) {
                    if (macro.parameters.size() != args.size())
                        return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " + std::to_string(macro.parameters.size()), cur.src };
                    std::vector<Line> body;
                    for (const auto& body_line : macro.body)
                        body.push_back({ substitute_params(body_line, macro.parameters, args), cur.src,
                            cur.depth + 1, cur.chain, cur.file });
                    push_front_lines(work, body);
                    continue;
                }
            }

            out.push_back(line);
            line_map.push_back({ static_cast<uint32_t>(cur.file), static_cast<uint32_t>(cur.src) });
        }
        return { };
    }

    ErrorInfo preprocess(std::string& file, RecursionHandler RH = RecursionHandler(),
        const std::string& name = "main") {
        file_names.clear();
        line_map.clear();
        const size_t main_id = file_id(name);
        const std::string chain = "|" + name + "|";

        const std::vector<std::string> raw = split_lines(file);
        std::deque<Line> work;
        for (size_t i = 0; i < raw.size(); i++)
            work.push_back({ raw[i], i, 0, chain, main_id });

        std::vector<std::string> out;
        ErrorInfo e = process(work, out, RH);
        if (e.code != ErrorCode::OK) {
            e.file = file_names[last_file];
            return e;
        }

        file.clear();
        for (const auto& line : out)
            file += line + "\n";
        return { };
    }
};

#endif