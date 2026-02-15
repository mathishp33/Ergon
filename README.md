
[ASM interpreter](#asm-interpreter)

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
* a <a href="#ASM interpreter">ASM interpreter</a> to convert assembly code to 32-bit bytecode

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

    ADD     = 0x00, // R-type: ADD rd, rs1, rs2
    SUB     = 0x01, // R-type: SUB rd, rs1, rs2
    MUL     = 0x02, // R-type: MUL rd, rs1, rs2
    DIV     = 0x03, // R-type: DIV rd, rs1, rs2
    MOD     = 0x04, // R-type: MOD rd, rs1, rs2
    ADDI    = 0x05, // I-type: ADDI rd, rs, imm
    SUBI    = 0x06, // I-type: SUBI rd, rs, imm
    MULI    = 0x07, // I-type: MULI rd, rs, imm
    DIVI    = 0x08, // I-type: DIVI rd, rs, imm
    MODI    = 0x09, // I-type: MODI rd, rs, imm

    AND     = 0x0A, // R-type: AND rd, rs1, rs2
    OR      = 0x0B, // R-type: OR rd, rs1, rs2
    XOR     = 0x0C, // R-type: XOR rd, rs1, rs2
    ANDI    = 0x0D, // I-type: ANDI rd, rs, imm
    ORI     = 0x0E, // I-type: ORI rd, rs, imm
    XORI    = 0x0F, // I-type: XORI rd, rs, imm

    SHL     = 0x10, // R-type: SHL rd, rs, rt
    SHR     = 0x11, // R-type: SHR rd, rs, rt
    SAR     = 0x12, // R-type: SAR rd, rs, rt
    ROL     = 0x13, // R-type: ROL rd, rs, rt
    ROR     = 0x14, // R-type: ROR rd, rs, rt
    SHLI    = 0x15, // I-type: SHLI rd, rs, imm
    SHRI    = 0x16, // I-type: SHRI rd, rs, imm
    SARI    = 0x17, // I-type: SARI rd, rs, imm
    ROLI    = 0x18, // I-type: ROLI rd, rs, imm
    RORI    = 0x19, // I-type: RORI rd, rs, imm

    CMP     = 0x1A, // R-type: CMP rs1, rs2
    TEST    = 0x1B, // R-type: TEST rs1, rs2
    CMPI    = 0x1C, // I-type: CMPI rs, imm
    TESTI   = 0x1D, // I-type: TESTI rs, imm

    INC     = 0x1E, // J-type: INC rd
    DEC     = 0x1F, // J-type: DEC rd
    NOT     = 0x20, // J-type: NOT rd, rs
    ABS     = 0x21, // J-type: ABS rd, rs
    NEG     = 0x22, // J-type: NEG rd, rs
    MIN     = 0x23, // R-type: MIN rd, rs1, rs2
    MAX     = 0x24, // R-type: MAX rd, rs1, rs2
    MINI    = 0x25, // I-type: MINI rd, rs1, imm
    MAXI    = 0x26, // I-type: MAXI rd, rs1, imm

    MOV_IMM = 0x27, // MOV rd, imm
    MOV_REG = 0x28, // MOV rd, rs1
    LOAD    = 0x29, // LOAD rd, [imm]
    STORE   = 0x2A, // STORE rs1, [imm]
    LOAD_BASE = 0x2B, // LOAD rd, [rs1 + imm]
    LDB     = 0x2C, // LOAD byte rd, [rs1 + imm] 8b sign-extended
    LDH     = 0x2D, // LOAD halfword rd, [rs1 + imm] 16b sign-extended
    LDW     = 0x2E, // LOAD word rd, [rs1 + imm] 32b
    STORE_BASE = 0x2F, // STORE rs1, [rd + imm]
    STB     = 0x30, // STORE byte rs1, [rd + imm] 8b
    STH     = 0x31, // STORE halfword rs1, [rd + imm] 16b
    STW     = 0x32, // STORE word rs1, [rd + imm] 32b
    PUSH    = 0x33, // PUSH rs1
    POP     = 0x34, // POP rd
    LEA     = 0x35, // LEA rd, rs1, imm
    SWAP    = 0x36, // SWAP rd, rs1
    CLR     = 0x37, // CLR rd
    MEMCPY  = 0x38, // MEMCPY rd, rs1, imm (length = imm)

    JMP     = 0x39, // JMP {(rd, rs1, rs2) = 24b} (RELATIVE JUMP)
    JZ      = 0x3A, // JZ {(rd, rs1, rs2) = 24b} (JMPR if Z flag is true)
    JNZ     = 0x3B, // JNZ {(rd, rs1, rs2) = 24b} (JMPR if Z flag is false)
    JG      = 0x3C, // JG {(rd, rs1, rs2) = 24b} (JMPR if both Z and N flag are false)
    JL      = 0x3D, // JL {(rd, rs1, rs2) = 24b} (JMPR if N flag is true)
    CALL    = 0x3E, // CALL {(rd, rs1, rs2) = 24b}  (JMPR and saves the current PC)
    RET     = 0x3F, // RET (load previous PC and JMPR there)

    HALT    = 0xFF // HALT (stops program)
    

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

### ASM interpreter

The ASM interpreter translate Assembly code into 32-bit bytecode (ex: add 4 6 7 -> 0x00040607).

The interpreter operates this way:
* it slices the program in lines
* it trims spaces and get rid of comments
* for each line it:
    * checks if a label is declared
    * it not, it translates the line into bytecode

The interpreter has specific error codes:
* 0 = no error
* 1 = unknown instruction
* 2 = invalid argument for instruction
* 3 = unknown label
* 4 = invalid character (stoi error)
* 5 = overflow (stoi error)
