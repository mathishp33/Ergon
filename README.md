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

| regs | f-regs |
|------|--------|
| r0 | f0 |
| r1	| f1 |
| r2	| f2 |
| r3	| f3 |
| r4	| f4 |
| r5	| f5 |
| r6	| f6 |
| r7	| f7 |
| r8	| f8 |
| r9	| f9 |
| r10	| f10 |
| r11	| f11 |
| r12	| f12 |
| r13	| f13 |
| r14	| f14 |
| r15	| f15 |


### Assembler

The assembler is composed of: 

ASM source code ─► Assembler (per file)

Assembler (per file) ─► ObjectFile (.o-like)

ObjectFile (.o-like) ─► Linker (multi-file, static libs)
 
Linker (multi-file, static libs) ─► Executable image (ELF-like)

Executable (ELF-like) ─► Loader (ROM)
   
Loader (ROM) ─► Interpreter (CPU level)

