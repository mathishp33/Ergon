# Structure
Ergon is composed of:
* <a href="#Hephaistos">Hephaistos</a> is a OS creation tool with its dedicated virtual environment
* <a href="#Talos">Talos</a> is a virtual asm IDE with dedicated virtual environment simpler than Hephaistos

# Hephaistos



# Talos

Talos is composed of:
* a Mother Board
  * a 32-bit <a href="#CPU">CPU</a>
    * single <a href="#Core">Core</a>
        * <a href="#ALU">ALU</a>
        * 16 registers
  * a from 1 to 516Kb RAM
  * a ROM where all the instructions (32-bit) are stored
* an <a href="#Assembler">Assembler</a> mixing NASM and ARM syntax.

## Architecture

### ALU

ALU Operations:
* ADD
* SUB
* MUL
* DIV
* MOD
* AND
* OR
* XOR
* NOT
* SHL
* SHR
* SAR
* ROL
* ROR
* CMP
* TES
* INC
* DEC
* MIN
* MAX
* ABS
* NEG

ALU Flags:
* Zero
* Negative
* Carry
* Overflow

### Core

Core Instructions (type R, I and J):

<b>Note that all instruction must be written in lower case</b>

    //----------------- ALU OPERATIONS -----------------
    ADD  , // R-type: add rd, rs1, rs2
    SUB  , // R-type: sub rd, rs1, rs2
    MUL  , // R-type: mul rd, rs1, rs2
    DIV  , // R-type: div rd, rs1, rs2
    MOD  , // R-type: mod rd, rs1, rs2
    ADDI , // I-type: addi rd, rs, imm
    SUBI , // I-type: subi rd, rs, imm
    MULI , // I-type: muli rd, rs, imm
    DIVI , // I-type: divi rd, rs, imm
    MODI , // I-type: modi rd, rs, imm

    AND  , // R-type: and rd, rs1, rs2
    OR   , // R-type: or rd, rs1, rs2
    XOR  , // R-type: xor rd, rs1, rs2
    ANDI , // I-type: andi rd, rs, imm
    ORI  , // I-type: ori rd, rs, imm
    XORI , // I-type: xori rd, rs, imm

    SHL  , // R-type: shl rd, rs, rt
    SHR  , // R-type: shr rd, rs, rt
    SAR  , // R-type: sar rd, rs, rt
    ROL  , // R-type: rol rd, rs, rt
    ROR  , // R-type: ror rd, rs, rt
    SHLI , // I-type: shli rd, rs, imm
    SHRI , // I-type: shri rd, rs, imm
    SARI , // I-type: sari rd, rs, imm
    ROLI , // I-type: roli rd, rs, imm
    RORI , // I-type: rori rd, rs, imm

    CMP  , // R-type: cmp rs1, rs2
    CMPU , // R-type: cmpu rs1, rs2
    TEST , // R-type: test rs1, rs2
    CMPI , // I-type: cmpi rs, imm
    CMPUI, // I-type: cmpui rs, imm (unsigned)
    TESTI, // I-type: testI rs, imm

    INC , // J-type: inc rd
    DEC , // J-type: dec rd
    NOT , // J-type: not rd, rs
    ABS , // J-type: abs rd, rs
    NEG , // J-type: neg rd, rs
    MIN , // R-type: min rd, rs1, rs2
    MAX , // R-type: max rd, rs1, rs2
    MINI, // I-type: mini rd, rs1, imm
    MAXI, // I-type: maxi rd, rs1, imm

    //----------------- FPU OPERATIONS -----------------

    AFAIRE0,
    AFAIRE1,
    AFAIRE2,
    AFAIRE3,
    AFAIRE4,
    AFAIRE5,
    AFAIRE6,

    //----------------- MEMORY OPERATIONS -----------------
    MOV_IMM , // mov rd, imm
    MOV_REG , // mov rd, rs1
    LDB_ABS , // load byte rd, [imm]
    LDH_ABS , // load half-word rd, [imm] (16b)
    LDW_ABS , // load word rd, [imm] (16b)
    STB_ABS , // store byte rd, [imm] (16b)
    STH_ABS , // store half-word rd, [imm] (16b)
    STW_ABS , // store word rd, [imm] (16b)
    LDB_BASE, // load byte rd, [rs1 + imm]
    LDH_BASE, // load half-word rd, [rs1 + imm]
    LDW_BASE, // load word rd, [rs1 + imm]
    STB_BASE, // store byte rd, [rs1 + imm]
    STH_BASE, // store half-word rd, [rs1 + imm]
    STW_BASE, // store word rd, [rs1 + imm]
    PUSH    , // push rs1
    POP     , // pop rd
    LEA     , // lea rd, rs1, imm
    SWAP    , // swap rd, rs1
    CLR     , // clr rd
    MEMCPY  , // memcpy rd, rs1, imm (length = imm)

    //----------------- PROGRAM OPERATIONS -----------------
    JMP , // jmp label (24b) (RELATIVE JUMP)
    JZ  , // jz label (24b) (JMPR if Z flag is true)
    JNZ , // jnz label (24b) (JMPR if Z flag is false)
    JG  , // jg label (24b) (JMPR if both Z and N flag are false)
    JL  , // jl label (24b) (JMPR if N flag is true)
    CALL, // call label (24b)  (JMPR and saves the current PC)
    RET , // ret (load previous PC and JMPR there)

    HALT // halt (stops program)
    

### CPU

The CPU is composed of a single Core:

```
 ┌──────────CPU──────────┐
 │                       │    ┌───┐
 │                 ┌───┐ ◄────►RAM│
 │                 │ALU│ │    └───┘
 │                 └─▲─┘ │
 │                   │   │
 │┌───────────┐   ┌──▼─┐ │    ┌───┐
 ││ registers │◄──┤Core│ │◄───┤ROM│
 │└───────────┘   └────┘ │    └───┘
 └───────────────────────┘
```

### Registers

The CPU has 16 (32-bit) registers:

| Index | Name | Purpose |
|----|------|----------|
| 0 | zero | General-purpose, used for cache |
| 1	| eax	|	General-purpose, accumulator |
| 2	| ebx	|	General-purpose, base |
| 3	| ecx	|	Counter / loop index |
| 4	| edx	|	Data / temp |
| 5	| esi	|	Source index / arg |
| 6	| edi	|	Destination index / arg |
| 7	| r8	| Scratch / temp |
| 8	| r9	| Scratch / temp |
| 9	| r10	|	Preserved / callee-saved |
| 10	| r11	|	Preserved / callee-saved |
| 11	| r12	|	Preserved / callee-saved |
| 12	| r13	|	Frame pointer (optional) |
| 13	| sp	|	Stack pointer |
| 14	| bp	| Base pointer |
| 15	| r15	|	Extra general-purpose |


### Assembler

The assembler is composed of: 

ASM source code ─► Assembler (per file)

Assembler (per file) ─► ObjectFile (.o-like)

ObjectFile (.o-like) ─► Linker (multi-file, static libs)
 
Linker (multi-file, static libs) ─► Executable image (ELF-like)

Executable (ELF-like) ─► Loader (ROM)
   
Loader (ROM) ─► Interpreter (CPU level)

