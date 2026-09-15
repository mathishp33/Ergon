# HELP

The Ergon's assembly language is a mix of x86 asm, ARM asm and my own syntax.

Let's begin with the basics: 

## Index

- [Registers](#registers)
- [Sections](#sections)
- [Instructions](#instructions)
- [Variables](#variables)
- [Preprocessor](#preprocessor)
- [System Calls](#system-calls)

## CPU specifications



## Registers

| regs | purpose              | f-regs |
|------|----------------------|--------|
| r0   | syscall              | f0     |
| r1	  | arg 0                | f1     |
| r2	  | arg 1                | f2     |
| r3	  | arg 2                | f3     |
| r4	  | return value         | f4     |
| r5	  | general purpose/temp | f5     |
| r6	  | general purpose/temp | f6     |
| r7	  | general purpose/temp | f7     |
| r8	  | general purpose/temp | f8     |
| r9	  | general purpose/temp | f9     |
| r10	 | general purpose/temp | f10    |
| r11	 | general purpose/temp | f11    |
| tmp	 | temp                 | f12    |
| cmp	 | cmp                  | f13    |
| sp	  | SP                   | f14    |
| pc   | PC                   | f15    |

## Sections

There are 3 sections:
 - ``.text`` used to declare instructions
 - ``.data`` used to declare initialized data
 - ``.rodata`` used to declare constant data
- ``.bss`` used to declare uninitialized data

## Instructions

Here is a complete list of all the instruction:

// { <instruction_name>, { <opcode>, <instruction_tpye>, { <argument_type>, ... } } }

| name    | type   | arguments     | equivalent                     |
|---------|--------|---------------|--------------------------------|
| add     | Type-R | REG, REG, REG | rd = rs1 + rs2                 |
| sub     | Type-R | REG, REG, REG | rd = rs1 - rs2                 |
| mul     | Type-R | REG, REG, REG | rd = rs1 * rs2                 |
| div     | Type-R | REG, REG, REG | rd = rs1 / rs2                 |
| mod     | Type-R | REG, REG, REG | rd = rs1 % rs2                 |
| addi    | Type-I | REG, REG, IMM | rd = rs1 + imm                 |
| subi    | Type-I | REG, REG, IMM | rd = rs1 - imm                 |
| muli    | Type-I | REG, REG, IMM | rd = rs1 * imm                 |
| divi    | Type-I | REG, REG, IMM | rd = rs1 / imm                 |
| modi    | Type-I | REG, REG, IMM | rd = rs1 % imm                 |
| and     | Type-R | REG, REG, REG | rd = rs1 & rs2                 |
| or      | Type-R | REG, REG, REG | rd = rs1 \| rs2                |
| xor     | Type-R | REG, REG, REG | rd = rs1 ^ rs2                 |
| andi    | Type-I | REG, REG, IMM | rd = rs1 & imm                 |
| ori     | Type-I | REG, REG, IMM | rd = rs1 \| imm                |
| xori    | Type-I | REG, REG, IMM | rd = rs1 ^ imm                 |
| shl     | Type-R | REG, REG, REG | rd = rs1 << rs2                |
| shr     | Type-R | REG, REG, REG | rd = rs1 >> rs2                |
| sar     | Type-R | REG, REG, REG | rd = rs1 >> rs2 (signed)       |
| rol     | Type-R | REG, REG, REG | rd = rotl(rs1, rs2)            |
| ror     | Type-R | REG, REG, REG | rd = rotr(rs1, rs2)            |
| shli    | Type-I | REG, REG, IMM | rd = rs1 << imm                |
| shri    | Type-I | REG, REG, IMM | rd = rs1 >> imm                |
| sari    | Type-I | REG, REG, IMM | rd = rs1 >> imm (signed)       |
| roli    | Type-I | REG, REG, IMM | rd = rotl(rs1, imm)            |
| rori    | Type-I | REG, REG, IMM | rd = rotr(rs1, imm)            |
| cmp     | Type-R | REG, REG      | cmp = rs1 ? rs2                |
| cmpu    | Type-R | REG, REG      | cmp = rs1 ? rs2 (unsigned)     |
| test    | Type-R | REG, REG      | cmp = rs1 & rs2                |
| cmpi    | Type-I | REG, IMM      | cmp = rs1 ? imm                |
| cmpui   | Type-I | REG, IMM      | cmp = rs1 ? imm (unsigned)     |
| testi   | Type-I | REG, IMM      | cmp = rs1 & imm                |
| inc     | Type-J | REG           | rd = rd + 1 (rd++)             |
| dec     | Type-J | REG           | rd = rd - 1 (rd--)             |
| not     | Type-J | REG, REG      | rd = !rs1                      |
| abs     | Type-J | REG, REG      | rd = abs(rs1)                  |
| neg     | Type-J | REG, REG      | rd = -rs1                      |
| min     | Type-R | REG, REG, REG | rd = min(rs1, rs2)             |
| max     | Type-R | REG, REG, REG | rd = max(rs1, rs2)             |
| mini    | Type-I | REG, REG, IMM | rd = min(rs1, imm)             |
| maxi    | Type-I | REG, REG, IMM | rd = max(rs1, imm)             |
| mov     | Type-R | REG, REG      | rd = rs1                       |
| movi    | Type-I | REG, IMM      | rd = imm                       |
| lbaseb  | Type-I | REG, REG, IMM | rd = load8(rs1 + imm)          |
| lbaseh  | Type-I | REG, REG, IMM | rd = load16(rs1 + imm)         |
| lbasew  | Type-I | REG, REG, IMM | rd = load32(rs1 + imm)         |
| lregb   | Type-R | REG, REG, REG | rd = load8(rs1 + rs2)          |
| lregh   | Type-R | REG, REG, REG | rd = load16(rs1 + rs2)         |
| lregw   | Type-R | REG, REG, REG | rd = load32(rs1 + rs2)         |
| sbaseb  | Type-I | REG, REG, IMM | rd = store8(rs1 + imm)         |
| sbaseh  | Type-I | REG, REG, IMM | rd = store16(rs1 + imm)        |
| sbasew  | Type-I | REG, REG, IMM | rd = store32(rs1 + imm)        |
| sregb   | Type-R | REG, REG, REG | rd = store8(rs1 + rs2)         |
| sregh   | Type-R | REG, REG, REG | rd = store16(rs1 + rs2)        |
| sregw   | Type-R | REG, REG, REG | rd = store32(rs1 + rs2)        |
| ldb     | Type-I | REG, VAR      | rd = load8(var_addr)           |
| ldh     | Type-I | REG, VAR      | rd = load16(var_addr)          |
| ldw     | Type-I | REG, VAR      | rd = load32(var_addr)          |
| stb     | Type-I | REG, VAR      | rd = store8(var_addr)          |
| sth     | Type-I | REG, VAR      | rd = store16(var_addr)         |
| stw     | Type-I | REG, VAR      | rd = store32(var_addr)         |
| push    | Type-J | REG           |                                |
| pop     | Type-J | REG           |                                |
| lea     | Type-I | REG, VAR, IMM | rd = var_addr + imm            |
| leab    | Type-I | REG, VAR      | rd = var_addr                  |
| swap    | Type-R | REG, REG      | rd, rs1 = rs1, rd              |
| clr     | Type-J | REG           | rd = 0                         |
| memcpy  | Type-I | REG           |                                |
| jmp     | Type-J | LABEL         | jump label                     |
| jz      | Type-J | LABEL         | jz (jump if flag Z) label      |
| jnz     | Type-J | LABEL         | jnz (jump if not flag Z) label |
| jg      | Type-J | LABEL         | jg (...) label                 |
| jl      | Type-J | LABEL         | jl (...) label                 |
| call    | Type-J | LABEL         | call label                     |
| ret     | Type-J |               | return from label              |
| syscall | Type-J |               | calls a system call            |
| halt    | Type-J |               | halts the program              |


instructions are placed inside the ``.text`` section with the following syntax:
```
.section .text:
  inc r0
  addi r0, r0, 0x54
  mul r1, r0, r0

```

## Variables

The basic syntax to declare variables is: 
````
.section .data:
  my_variable:
    .byte 0x1
````

There is multiples variables types in assembly: 
- ``.byte`` a uint8(char)
- ``.hword`` a uint16(short)
- ``.word`` a uint32(int)

you can declare arrays too:
````
.section .data:
  first_array:
    .byte 'a', ' ', 's', 't', 'r', 'i', 'n', 'g'
  sec_array:
    .zero 4 ; array of 4 bytes of zeros
````

## Preprocessor

The preprocessor a useful tool to use especially when programming in assembly.

| name    | description                  | example                                | 
|---------|------------------------------|----------------------------------------|
| %equ    | declares a constant          | %equ my_cst 43                         |
| %assign | declares a variable          | %assign i 43 + a                       |
| %define | declares a text substitution | %define DINC(reg) addi reg reg 2       |
| %macro  | declares a macro             | %macro RET() <br/> ret <br/> %endmacro |
| %rep    | declares a loop              | %rep 8 <br/> inc r0 <br/> %endrep      |

## System Calls

You can call ``syscall`` to:
 - 0 = exit()
 - 1 = write(buff, size)
 - 2 = read(buff, size)
