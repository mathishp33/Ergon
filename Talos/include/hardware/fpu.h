#ifndef ERGON_FPU_H
#define ERGON_FPU_H


enum class FPUOp {
    FADD, // a + b
    FSUB, // a - b
    FMUL, // a * b
    FDIV, // a / b
    FMA, // (a * b) + c

    FSQRT, // sqrt(a)
    FABS, // abs(a)
    FNEG, // -a

    FCMP, // a ? b

    ITOF, // int -> float
    FTOI // float -> int
};

#endif