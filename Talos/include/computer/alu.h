#ifndef ERGON_ALU_H
#define ERGON_ALU_H


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


#endif