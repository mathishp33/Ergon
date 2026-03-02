# Structure
Ergon is composed of:
* <a href="#Hephaistos">Hephaistos</a> is a OS creation tool with its dedicated virtual environment
* <a href="#Talos">Talos</a> is a virtual environment with its own 32 bit architecture (CPU, ASM interpreter & syntax, ...)

# Hephaistos



# Talos

## Usage

Talos is a headers-only librairy that can only be compiled by Clang or GCC.

First, declare an Environment Manager: 
```
EnvironmentManager env = EnvironmentManager();
```
Then, write an asm program:
```
std::string program = "section .text \n"
                      " ldw ebx var \n"
                      " loop: \n"
                      "  inc eax \n"
                      "  cmp eax ebx \n"
                      "  jl loop \n"
                      "section .data \n"
                      " var dd 20_000_000 \n";
```
Build:
```
std::cout << env.build({ { "my_progam", program } }) << std::endl;
```
Or for one file:
```
std::cout << env.build_single(program) << std::endl;
```
There is two execution modes:
* AUTO runs the program normally (fast, simple)
  ```
  env.start();
  ```
* STEP runs a single line and returns  (easy to understand, debug)
  ```
  StepInfo current_step = env.step()
  ```

## Architecture

Talos is composed of:
* a Mother Board
  * a 32-bit <a href="#CPU">CPU</a>
    * single <a href="#Core">Core</a>
        * <a href="#ALU">ALU</a>
        * 16 registers
  * a from 1 to 516Kb RAM
  * a ROM where all the instructions (32-bit) are stored
* an <a href="#Assembler">Assembler</a> mixing NASM and ARM syntax.

### CPU

The CPU is composed of a single Core:

```
 ┌────────MOTHER-BOARD────────────┐
 │ ┌─────CPU────────────┐         │
 │ │ ┌───CORE─────────┐ │         │
 │ │ │ ┌──ALU───┐     │ │         │
 │ │ │ │┌──────┐│     │ │         │
 │ │ │ ││regs  ││     │ │    ┌───┐│
 │ │ │ │└──────┘│     │ │◄──►│RAM││
 │ │ │ │┌──────┐│     │ │    └───┘│
 │ │ │ ││ALU-op││     │ │         │
 │ │ │ │└──────┘│     │ │         │
 │ │ │ │┌──────┐│     │ │    ┌───┐│
 │ │ │ ││Flags ││     │ │◄───┤ROM││
 │ │ │ │└──────┘│     │ │    └───┘│
 │ │ │ └────────┘     │ │         │
 │ │ │ ┌──FPU───┐     │ │         │
 │ │ │ │┌──────┐│     │ │         │
 │ │ │ ││f-regs││     │ │         │
 │ │ │ │└──────┘│     │ │         │
 │ │ │ │┌──────┐│     │ │         │
 │ │ │ ││FPU-op││┌──┐ │ │         │
 │ │ │ │└──────┘││SP│ │ │         │
 │ │ │ │┌──────┐│└──┘ │ │         │
 │ │ │ ││Flags ││┌──┐ │ │         │
 │ │ │ │└──────┘││PC│ │ │         │
 │ │ │ └────────┘└──┘ │ │         │
 │ │ └────────────────┘ │         │
 │ └────────────────────┘         │
 └────────────────────────────────┘
```


### Core

A core has:
* 16 general-purpose (32-bit) registers
* 16 float registers
* a modifiable (&regs[15]) Stack Pointer (SP)
* a Program Counter
* ALU Flags
* FPU Flags
* an access to the Program ROM
* an access to the RAM

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


### Registers

| regs  | f-regs |
|-------|--------|
| r0    | f0     |
| r1	   | f1     |
| r2	   | f2     |
| r3	   | f3     |
| r4	   | f4     |
| r5	   | f5     |
| r6	   | f6     |
| r7	   | f7     |
| r8	   | f8     |
| r9	   | f9     |
| r10	  | f10    |
| r11	  | f11    |
| tmp	  | f12    |
| cmp	  | f13    |
| sp	   | f14    |
| zero	 | f15    |


### Assembler

The assembler is composed of: 
\
ASM source code ─► Assembler (per file)
\
Assembler (per file) ─► ObjectFile (.o-like)
\
ObjectFile (.o-like) ─► Linker (multi-file, static libs)
\
Linker (multi-file, static libs) ─► Executable image (ELF-like)
\
Executable (ELF-like) ─► Loader (ROM)
\
Loader (ROM) ─► Interpreter (CPU level)

Here is a complete list of all the instruction: 

// { <instruction_name>, { <opcode>, <instruction_tpye>, { <argument_type>, ... } } }

| name   | type   | arguments     | equivalent                     |
|--------|--------|---------------|--------------------------------|
| add    | Type-R | REG, REG, REG | rd = rs1 + rs2                 |
| sub    | Type-R | REG, REG, REG | rd = rs1 - rs2                 |
| mul    | Type-R | REG, REG, REG | rd = rs1 * rs2                 |
| div    | Type-R | REG, REG, REG | rd = rs1 / rs2                 |
| mod    | Type-R | REG, REG, REG | rd = rs1 % rs2                 |
| addi   | Type-I | REG, REG, IMM | rd = rs1 + imm                 |
| subi   | Type-I | REG, REG, IMM | rd = rs1 - imm                 |
| muli   | Type-I | REG, REG, IMM | rd = rs1 * imm                 |
| divi   | Type-I | REG, REG, IMM | rd = rs1 / imm                 |
| modi   | Type-I | REG, REG, IMM | rd = rs1 % imm                 |
| and    | Type-R | REG, REG, REG | rd = rs1 & rs2                 |
| or     | Type-R | REG, REG, REG | rd = rs1 \| rs2                |
| xor    | Type-R | REG, REG, REG | rd = rs1 ^ rs2                 |
| andi   | Type-I | REG, REG, IMM | rd = rs1 & imm                 |
| ori    | Type-I | REG, REG, IMM | rd = rs1 \| imm                |
| xori   | Type-I | REG, REG, IMM | rd = rs1 ^ imm                 |
| shl    | Type-R | REG, REG, REG | rd = rs1 << rs2                |
| shr    | Type-R | REG, REG, REG | rd = rs1 >> rs2                |
| sar    | Type-R | REG, REG, REG | rd = rs1 >> rs2 (signed)       |
| rol    | Type-R | REG, REG, REG | rd = rotl(rs1, rs2)            |
| ror    | Type-R | REG, REG, REG | rd = rotr(rs1, rs2)            |
| shli   | Type-I | REG, REG, IMM | rd = rs1 << imm                |
| shri   | Type-I | REG, REG, IMM | rd = rs1 >> imm                |
| sari   | Type-I | REG, REG, IMM | rd = rs1 >> imm (signed)       |
| roli   | Type-I | REG, REG, IMM | rd = rotl(rs1, imm)            |
| rori   | Type-I | REG, REG, IMM | rd = rotr(rs1, imm)            |
| cmp    | Type-R | REG, REG      | flags = rs1 ? rs2              |
| cmpu   | Type-R | REG, REG      | flags = rs1 ? rs2 (unsigned)   |
| test   | Type-R | REG, REG      | flags = rs1 & rs2              |
| cmpi   | Type-I | REG, IMM      | flags = rs1 ? imm              |
| cmpui  | Type-I | REG, IMM      | flags = rs1 ? imm (unsigned)   |
| testi  | Type-I | REG, IMM      | flags = rs1 & imm              |
| inc    | Type-J | REG           | rd = rd + 1 (rd++)             |
| dec    | Type-J | REG           | rd = rd - 1 (rd--)             |
| not    | Type-J | REG, REG      | rd = !rs1                      |
| abs    | Type-J | REG, REG      | rd = abs(rs1)                  |
| neg    | Type-J | REG, REG      | rd = -rs1                      |
| min    | Type-R | REG, REG, REG | rd = min(rs1, rs2)             |
| max    | Type-R | REG, REG, REG | rd = max(rs1, rs2)             |
| mini   | Type-I | REG, REG, IMM | rd = min(rs1, imm)             |
| maxi   | Type-I | REG, REG, IMM | rd = max(rs1, imm)             |
| mov    | Type-R | REG, REG      | rd = rs1                       |
| movi   | Type-I | REG, IMM      | rd = imm                       |
| lbaseb | Type-I | REG, REG, IMM | rd = load8(rs1 + imm)          |
| lbaseh | Type-I | REG, REG, IMM | rd = load16(rs1 + imm)         |
| lbasew | Type-I | REG, REG, IMM | rd = load32(rs1 + imm)         |
| sbaseb | Type-I | REG, REG, IMM | rd = store8(rs1 + imm)         |
| sbaseh | Type-I | REG, REG, IMM | rd = store16(rs1 + imm)        |
| sbasew | Type-I | REG, REG, IMM | rd = store32(rs1 + imm)        |
| ldb    | Type-I | REG, VAR      | rd = load8(var_addr)           |
| ldh    | Type-I | REG, VAR      | rd = load16(var_addr)          |
| ldw    | Type-I | REG, VAR      | rd = load32(var_addr)          |
| stb    | Type-I | REG, VAR      | rd = store8(var_addr)          |
| sth    | Type-I | REG, VAR      | rd = store16(var_addr)         |
| stw    | Type-I | REG, VAR      | rd = store32(var_addr)         |
| push   | Type-J | REG           |                                |
| pop    | Type-J | REG           |                                |
| lea    | Type-I | REG, REG, IMM |                                |
| swap   | Type-R | REG, REG      |                                |
| clr    | Type-J | REG           |                                |
| memcpy | Type-I | REG           |                                |
| jmp    | Type-J | LABEL         | jump label                     |
| jz     | Type-J | LABEL         | jz (jump if flag Z) label      |
| jnz    | Type-J | LABEL         | jnz (jump if not flag Z) label |
| jg     | Type-J | LABEL         | jg (...) label                 |
| jl     | Type-J | LABEL         | jl (...) label                 |
| call   | Type-J | LABEL         |                                |
| ret    | Type-J |               |                                |
| halt   | Type-J |               | halt the program               |