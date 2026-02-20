#ifndef ERGON_ALU_H
#define ERGON_ALU_H

#include <cstdint>

struct Flags {
    bool Z = false; // Zero
    bool N = false; // Negative
    bool C = false; // Carry
    bool V = false; // Overflow
};

enum class ALUOp {
    ADD, // a + b
    SUB, // a - b
    MUL, // a * b
    DIV, // a / b
    MOD, // a % b

    AND, // a & b
    OR, // a | b
    XOR, // (a | b) & !(a & b)
    NOT, // !a

    SHL, // a << b bit shift left
    SHR, // a >> b bit shift right
    SAR, // bit shift right but it conserves sign
    ROL, // bit rotation left
    ROR, // bit rotation right

    CMP, // a ? b sets flags only
    CMPU, // a ? b sets flags only (unsinged)
    TEST, // a & b flags only

    INC, // a + 1
    DEC, // b - 1
    MIN, // min(a, b)
    MAX, // max(a, b)
    ABS, // abs(a)
    NEG // -a
};

// there is padding, but I don't want to #pragma pack(1) bc it gives warning
struct ALUResult {
    uint32_t value; // Result of the operation
    Flags flags; // Operations Flags
    bool writeback = true; // for CMP and TEST
    bool trap = false; // for forbidden operations
};

struct ALU {
    ALUResult exec(ALUOp op, uint32_t a, uint32_t b) {
        ALUResult r{};
        uint64_t u_wide; //unsigned wide
        int64_t  s_wide; //signed wide

        auto s_a = static_cast<int32_t>(a); //signed a
        auto s_b = static_cast<int32_t>(b); //signed b

        switch (op) {

        // Arithmetic
        case ALUOp::ADD:
            u_wide = static_cast<uint64_t>(a) + static_cast<uint64_t>(b);
            s_wide = static_cast<int64_t>(s_a) + static_cast<int64_t>(s_b);
            r.value = static_cast<uint32_t>(u_wide);
            r.flags.C = u_wide > 0xFFFFFFFF;
            r.flags.V = s_wide > INT32_MAX || s_wide < INT32_MIN;
            break;
        case ALUOp::SUB:
            u_wide = static_cast<uint64_t>(a) - static_cast<uint64_t>(b);
            s_wide = static_cast<int64_t>(s_a) - static_cast<int64_t>(s_b);
            r.value = static_cast<uint32_t>(u_wide);
            r.flags.C = a >= b;
            r.flags.V = s_wide > INT32_MAX || s_wide < INT32_MIN;
            break;
        case ALUOp::MUL:
            s_wide = static_cast<int64_t>(s_a) * static_cast<int64_t>(s_b);
            r.value = static_cast<uint32_t>(s_wide);
            r.flags.V = s_wide > INT32_MAX || s_wide < INT32_MIN;
            break;
        case ALUOp::DIV:
            if (s_b == 0 || (s_a == INT32_MIN && s_b == -1))
                r.trap = true;
            else
                r.value = static_cast<uint32_t>(s_a / s_b);
            break;
        case ALUOp::MOD:
            if (s_b == 0)
                r.trap = true;
            else
                r.value = static_cast<uint32_t>(s_a % s_b);
            break;

        // Logic
        case ALUOp::AND:
            r.value = a & b;
            break;
        case ALUOp::OR:
            r.value = a | b;
            break;
        case ALUOp::XOR:
            r.value = a ^ b;
            break;
        case ALUOp::NOT:
            r.value = ~a;
            break;

        // Shifts
        case ALUOp::SHL:
            r.value = a << (b & 31);
            r.flags.C = (a >> (32 - (b & 31))) & 1;
            break;
        case ALUOp::SHR:
            r.value = a >> (b & 31);
            r.flags.C = (a >> ((b & 31) - 1)) & 1;
            break;
        case ALUOp::SAR:
            r.value = static_cast<uint32_t>(s_a >> (b & 31));
            break;
        case ALUOp::ROL:
            r.value = (a << (b & 31)) | (a >> (32 - (b & 31)));
            break;
        case ALUOp::ROR:
            r.value = (a >> (b & 31)) | (a << (32 - (b & 31)));
            break;

        // Compare / Test
        case ALUOp::CMP:
            {
                int32_t diff = s_a - s_b;
                r.writeback = false;
                r.flags.Z = diff == 0;
                r.flags.N = diff < 0;
                r.flags.C = a >= b;
                r.flags.V = ((s_a ^ s_b) & (s_a ^ diff)) < 0;
                return r;
            }
        case ALUOp::CMPU:
                r.writeback = false;
                r.flags.Z = a == b;
                r.flags.N = a < b;
                r.flags.C = a >= b;
                r.flags.V = ((a ^ b) & (a ^ (a - b))) < 0;
                return r;

        case ALUOp::TEST:
            {
                uint32_t v = a & b;
                r.writeback = false;
                r.flags.Z = (v == 0);
                r.flags.N = (v >> 31) & 1;
                return r;
            }

        // Unary
        case ALUOp::INC:
            s_wide = static_cast<int64_t>(s_a) + 1;
            r.value = static_cast<uint32_t>(s_wide);
            r.flags.V = s_wide > INT32_MAX;
            break;
        case ALUOp::DEC:
            s_wide = static_cast<int64_t>(s_a) - 1;
            r.value = static_cast<uint32_t>(s_wide);
            r.flags.V = s_wide < INT32_MIN;
            break;
        case ALUOp::MIN:
            if (a < b)
                r.value = a;
            else
                r.value = b;
            break;
        case ALUOp::MAX:
            if (a > b)
                r.value = a;
            else
                r.value = b;
            break;
        case ALUOp::ABS:
            if (s_a < 0)
                r.value = static_cast<uint32_t>(-s_a);
            else
                r.value = a;
            break;
        case ALUOp::NEG:
            if (s_a == INT32_MIN)
                r.trap = true;
            else
                r.value = static_cast<uint32_t>(-s_a);
            break;


        //a rajouter (peu etre):
        // mettre le bit choisi à 1
        // mettre le bit choisi à 0
        // changer un bit
        default:
            break;
        }

        r.flags.Z = (r.value == 0);
        r.flags.N = (r.value >> 31) & 1;

        return r;
    }
};


#endif