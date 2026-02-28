#ifndef ERGON_FPU_H
#define ERGON_FPU_H

#include <cmath>
#include <cstdint>
#include <cstring>

struct FPUFlags {
    bool Z = false; // Zero
    bool N = false; // Negative
    bool I = false; // Invalid operation (nan)
    bool O = false; // Overflow (inf)
    bool U = false; // Underflow
    bool D = false; // Divide by zero
};

enum class FPUOp {
    FADD, // a + b
    FSUB, // a - b
    FMUL, // a * b
    FDIV, // a / b
    FMA, // (a * b) + c

    FSQRT, // sqrt(a)
    FABS, // abs(a)
    FNEG, // - a

    FCMP, // a ? b sets flags only

    ITOF, // int -> float
    FTOI // float -> int
};

struct FPUResult {
    uint32_t value; // bits IEEE754
    FPUFlags flags;
    bool writeback = true;
    bool trap = false;
};

inline float bits_to_float(uint32_t v) {
    float f;
    std::memcpy(&f, &v, sizeof(float));
    return f;
}

inline uint32_t float_to_bits(float f) {
    uint32_t v;
    std::memcpy(&v, &f, sizeof(uint32_t));
    return v;
}

static FPUResult exec(FPUOp op, uint32_t a, uint32_t b, uint32_t c = 0) {
    FPUResult r{};

    float fa = bits_to_float(a);
    float fb = bits_to_float(b);

    float fr = 0;

    switch (op) {
    case FPUOp::FADD:
        fr = fa + fb;
        break;

    case FPUOp::FSUB:
        fr = fa - fb;
        break;

    case FPUOp::FMUL:
        fr = fa * fb;
        break;

    case FPUOp::FDIV:
        if (fb == 0.0f) {
            r.flags.D = true;
            r.trap = true;
            return r;
        }
        fr = fa / fb;
        break;
    case FPUOp::FMA:
        {
            float fc = bits_to_float(c);
            fr = std::fma(fa, fb, fc);
            r.value = float_to_bits(fr);
            break;
        }

    case FPUOp::FSQRT:

    case FPUOp::FABS:

    case FPUOp::FNEG:

    case FPUOp::ITOF:
        fr = static_cast<float>(static_cast<int32_t>(a));
        break;

    case FPUOp::FTOI:
        r.value = static_cast<uint32_t>(static_cast<int32_t>(fa));
        return r;
    default: ;
    }

    if (std::isnan(fr)) r.flags.I = true;
    if (std::isinf(fr)) r.flags.O = true;

    r.value = float_to_bits(fr);
    r.flags.Z = (fr == 0.0f);
    r.flags.N = std::signbit(fr);

    return r;
}

#endif