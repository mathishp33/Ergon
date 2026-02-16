#ifndef ERGON_ERROR_H
#define ERGON_ERROR_H

#include <string>

enum class ErrorCode : uint8_t {
    OK,
    UNKNOWN_INSTR,
    INVALID_ARG,
    UNKNOWN_LABEL,
    INVALID_CHAR,
    STOI_OVERFLOW,
    LINE_OVERFLOW
};

// there is padding, but I don't want to #pragma pack(1) bc it gives warning
struct ErrorInfo {
    ErrorCode error_code = ErrorCode::OK;
    size_t index_line = 0;

    ErrorInfo() = default;
    ErrorInfo(ErrorCode e_code, size_t i) {
        error_code = e_code;
        index_line = i;
    }
};

inline std::string ErrorCode_to_String(ErrorInfo e) {
    switch (e.error_code) {
    case ErrorCode::OK:
        return "No Errors.";
    case ErrorCode::UNKNOWN_INSTR:
        return "Unknown instruction at line " + std::to_string(e.index_line) + ".";
    case ErrorCode::INVALID_ARG:
        return "Invalid argument at line " + std::to_string(e.index_line) + ".";
    case ErrorCode::UNKNOWN_LABEL:
        return "Unknown label at line " + std::to_string(e.index_line) + ".";
    case ErrorCode::INVALID_CHAR:
        return "Invalid character at line " + std::to_string(e.index_line) + ".";
    case ErrorCode::STOI_OVERFLOW:
        return "Overflow while trying to convert an argument in line " + std::to_string(e.index_line) + ".";
    case ErrorCode::LINE_OVERFLOW:
        return "Overflow in line " + std::to_string(e.index_line) + ".";
    }
    return "Error " + std::to_string(e.index_line) + ".";
}

#endif