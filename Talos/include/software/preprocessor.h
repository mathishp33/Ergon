#ifndef ERGON_PREPROCESSOR_H
#define ERGON_PREPROCESSOR_H



struct RecursionHandler {
    size_t rep_depth = 0;
    size_t macro_expansion = 0;

    bool handle() {
        rep_depth++;
        macro_expansion++;
        return rep_depth > 100 || macro_expansion > 100;
    }
};

struct PreProcesser {
    std::unordered_map<std::string, Value> constants; // %equ
    std::unordered_map<std::string, Value> variables; // %assign
    std::unordered_map<std::string, Define> defines; // %define
    std::unordered_map<std::string, Macro> macros; // %macro

    static std::string delimit_string(const std::string& s, size_t& idx, const std::function<bool(char, size_t)>& condition ) {
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

    static std::pair<ErrorInfo, std::string> find_block_end(const std::vector<std::string>& lines, size_t& idx, const std::string& begin_dir,
        const std::string& end_dir, ErrorInfo e_i) {
        size_t depth = 1;
        std::string out;
        for (; idx < lines.size(); idx++) {
            std::string local_line = string_utils::normalize(lines[idx]);
            if (local_line.starts_with(begin_dir))
                depth++;
            else if (local_line == end_dir) {
                depth--;
                if (depth == 0)
                    break;
            }
            out += lines[idx] + "\n";
        }
        e_i.index_line = idx;
        if (depth != 0)
            return { e_i, "" };
        return { { }, out };
    }

    ErrorInfo preprocess(std::string& file, RecursionHandler RH = RecursionHandler()) {
        std::vector<std::string> lines = string_utils::slice_str(file, '\n');
        if (RH.handle())
            return { ErrorCode::PREPROC_RECURSION, "infinite recursion in the preprocessor", 0 };
        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = string_utils::normalize(lines[i]);

            if (line.empty()) continue;

            //handle defines
            for (const auto& [name, define] : defines) {
                if (define.parameters.empty()) {
                    std::string line_copy = line;
                    string_utils::replace_string_as_token(line_copy, name, define.replacement);
                    lines[i] = line_copy;
                }
                else {
                    if (leading_identifier(line) == name) {
                        std::string line_copy = line;
                        if (string_utils::rep_counter(line, ')') != 1)
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        if (string_utils::rep_counter(line, '(') != 1)
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        if (line[name.size()] == '(') {
                            size_t idx = name.size() + 1;
                            std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                            std::vector<std::string> args = string_utils::slice_str(compacted_args, ',');
                            for (auto& arg : args)
                                arg = string_utils::remove_char(arg, ' ');
                            string_utils::replace_string_as_token(line_copy, line, define.replacement);
                            if (define.parameters.size() != args.size())
                                return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " + std::to_string(define.parameters.size()), i };
                            for (size_t j = 0; j < args.size(); j++)
                                string_utils::replace_string_as_token(line_copy, define.parameters[j], args[j]);
                            lines[i] = line_copy;
                        }
                        else {
                            return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                        }

                    }
                }
            }
            line = string_utils::normalize(lines[i]);
            for (const auto& [name, macro] : macros) {
                if (leading_identifier(line) == name) {
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (line[name.size()] == '(') {
                        size_t idx = name.size() + 1;
                        std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                        std::vector<std::string> args = string_utils::slice_str(compacted_args, ',');
                        for (auto& arg : args)
                            arg = string_utils::remove_char(arg, ' ');
                        if (macro.parameters.size() != args.size())
                            return { ErrorCode::INVALID_ARG_SIZE, "invalid arg size, expected " + std::to_string(macro.parameters.size()), i };
                        std::vector<std::string> body;
                        for (const auto& local_line : macro.body) {
                            std::string local_line_copy = local_line;
                            for (size_t j = 0; j < args.size(); j++)
                                string_utils::replace_string_as_token(local_line_copy, macro.parameters[j], args[j]);
                            body.emplace_back(local_line_copy);
                        }
                        lines[i].clear();
                        lines.insert(lines.begin() + i, body.begin(), body.end());
                        //i += body.size();
                        i--;
                        line = string_utils::normalize(lines[i]);
                    }
                    else {
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    }
                }
            }


            if (!line.starts_with("%")) continue;

            size_t idx = 0;
            std::string instr = delimit_string(line, idx, [](char c, size_t idx) { return c != ' '; });
            size_t instr_idx = idx++;

            size_t name_idx = instr_idx;
            std::string name;
            if (instr != "%rep") {
                name = delimit_string(line, idx, [](char c, size_t idx) { return c != ' ' && c != '('; });
                name_idx = idx++;
                if (!string_utils::check_cst_name(name))
                    return { ErrorCode::INVALID_NAME, "invalid name, expected only letters, numbers and '_' ", i };
            }

            if (instr == "%equ") {
                std::string expr = line.substr(name_idx);
                auto [e, value] = parse_expr(expr, constants, variables);
                if (e.code != ErrorCode::OK) return e;

                if (constants.contains(name))
                    return { ErrorCode::DUPLICATE_CONSTANT, "duplicate constant \"" + name + "\"", i };
                constants[name] = value;
                lines[i].clear();
            }
            if (instr == "%assign") {
                std::string expr = line.substr(name_idx);
                auto [e, value] = parse_expr(expr, constants, variables);
                if (e.code != ErrorCode::OK) return e;

                variables[name] = value;
                lines[i].clear();
            }
            if (instr == "%define") {
                std::vector<std::string> args;
                std::string replacement;
                if (line[idx - 1] == '(') { //with args
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };

                    std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                    args = string_utils::slice_str(compacted_args, ',');
                    for (auto& arg : args)
                        arg = string_utils::remove_char(arg, ' ');
                    replacement = line.substr(++idx);
                }
                else { //no args
                    replacement = line.substr(idx);
                }

                if (defines.contains(name))
                    return { ErrorCode::DUPLICATE_DEFINE, "duplicate define \"" + name + "\"", i };
                defines[name] = { args, replacement };
                lines[i].clear();
            }
            if (instr == "%macro") {
                std::vector<std::string> args;
                std::vector<std::string> body;
                if (line[idx - 1] == '(') { //with args
                    if (string_utils::rep_counter(line, ')') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                    if (string_utils::rep_counter(line, '(') != 1)
                        return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };

                    std::string compacted_args = delimit_string(line, idx, [](char c, size_t idx) { return c != ')'; });
                    args = string_utils::slice_str(compacted_args, ',');
                    for (auto& arg : args)
                        arg = string_utils::remove_char(arg, ' ');

                    size_t j = i + 1;
                    for (; ; j++) {
                        if (j > lines.size())
                            return { ErrorCode::MISSING_ENDMACRO, "missing an %endmacro", i };

                        if (string_utils::normalize(lines[j]).starts_with("%endmacro"))
                            break;
                        body.emplace_back(lines[j]);
                    }
                    for (size_t k = i; k < j + 1; k++)
                        lines[k].clear();
                    i = j;
                }
                else {
                    return { ErrorCode::MISMATCHED_PAR, "mismatched parenthesis", i };
                }

                if (macros.contains(name))
                    return { ErrorCode::DUPLICATE_MACRO, "duplicate macro \"" + name + "\"", i };
                macros[name] = { args, body };
            }
            if (instr == "%rep") {
                std::string expr = line.substr(name_idx);
                auto [e_0, value] = parse_expr(expr, constants, variables);
                if (e_0.code != ErrorCode::OK) return e_0;

                size_t j = i + 1;
                auto [e_1, body] = find_block_end(lines, j, "%rep ", "%endrep",
                    {ErrorCode::MISSING_ENDREP, "missing an %endrep", i });
                if (e_1.code != ErrorCode::OK) return e_1;

                std::vector<std::string> to_add;
                for (int k = 0; k < value; k++) {
                    std::string body_copy = body;
                    ErrorInfo e_i = preprocess(body_copy, RH);
                    if (e_i.code != ErrorCode::OK) {
                        e_i.index_line += i;
                        return e_i;
                    }
                    for (const auto& local_line : string_utils::slice_str(body_copy, '\n'))
                        to_add.push_back(local_line);
                }
                for (size_t k = i; k < j + 1; k++)
                    lines[k].clear();
                i = j + 1;
                lines.insert(lines.begin() + i, to_add.begin(), to_add.end());
                i += to_add.size();
                i--;
            }
            if (instr == "%if") {

            }
            if (instr == "%ifdef") {

            }


        }
        file.clear();
        for (const auto& line : lines)
            if (!line.empty())
                file += line + "\n";

        return { };
    }

    // a rajouter
    // %if
    // %ifdef

};


#endif
