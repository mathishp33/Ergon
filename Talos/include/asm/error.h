#ifndef ERGON_ERROR_H
#define ERGON_ERROR_H

#include <string>
#include <utility>

enum class ErrorCode : uint8_t {
    OK, //
    INVALID_BASE, //invalid base (base should be between 2 and 36)
    STOI_POS_OVERFLOW, //posivite overflow in stoi
    STOI_NEG_OVERFLOW, //negative overflow in stoi
    STOI_INVALID_CHAR, //invalid char in stoi
    INVALID_REG, //invalid register name " "
    INVALID_BYTE_SIZE, //invalid byte size
    DUPLICATE_GLOBAL_SYMBOL, //duplicate global symbol " "
    UNRESOLVED_EXTERN_SYMBOL, //unresolved extern symbol " "
    UNKNOWN_ENTRY_SYBOL, //unknown entry symbol " "
    NO_ENTRY_DEFINED, //no entry defined, cannot know what should the starting PC
    UNKNOWN_SYMBOL, //unknown symbol " "
    DUPLICATE_LABEL, //duplicate label " "
    INVALID_ARG_SIZE, //invalid arg size expected ...
    INVALID_ARG, //invalid arg ...
    VAR_OUTSIDE_RIGHT_SECTION, //variable is outside ...
    VAR_OUTSIDE_BSS, //variable is outside bss section
    RAM_OVERFLOW, //ram overflow

};

// there is padding, but I don't want to #pragma pack(1) bc it gives warning
struct ErrorInfo {
    ErrorCode code = ErrorCode::OK;
    size_t index_line = 0;
    std::string message;

    ErrorInfo() = default;
    ErrorInfo(ErrorCode e_code, size_t i) : code(e_code), index_line(i) {}
    ErrorInfo(ErrorCode e_code, std::string e_msg) : code(e_code), message(std::move(e_msg)) {}
    ErrorInfo(ErrorCode e_code, std::string e_msg, size_t i) : code(e_code), message(std::move(e_msg)), index_line(i) {}

};

#endif