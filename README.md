# Structure
Ergon is composed of:
* <a href="#Hephaistos">Hephaistos</a> is a OS creation tool with its dedicated virtual environment
* <a href="#Talos">Talos</a> is a virtual environment with its own 32 bit architecture (CPU, ASM interpreter & syntax, ...)

# Hephaistos



# Talos

## Help

If you need help regarding the assembly language click [here](HELP.md).

## Usage

Talos is a headers-only librairy that can only be compiled by Clang or GCC.

First, declare an Environment Manager: 
```
EnvironmentManager env = EnvironmentManager();
```
Then, write an asm program:
```
std::string program = "section .text \n"
                      " ldw r0 var \n"
                      " loop: \n"
                      "  inc r1 \n"
                      "  cmp r1 r0 \n"
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
* an <a href="#Assembler">Assembler</a> mixing x86 and ARM syntax.

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
* 12 general-purpose (32-bit) registers (12 + TMP, CMP, SP, Zero)
* 16 float registers
* a Program Counter
* ALU Flags
* FPU Flags
* access to the Program ROM
* access to the RAM

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


### Assembler

The assembler is composed of: 
\
ASM source code ─► Preprocessor + Assembler (per file)
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

### Parser 

The Preprocessor & the Assembler support an advanced parser for immediate values.

The parser is capable of:

 - Separate float and int parsing (32-bit)
 - Basics operations (+, -, *, /)
 - Advanced operations (%, **)
 - Comparisons (>, <, >=, <=, ==)
 - Bit operations (>>, <<, &, |, ^, ~)
 - Parenthesis
 - Supporting various inputs:
   - decimal
   - hexadecimal (0x)
   - binary (0b)
   - octet (0o)
   - char ('')
   - float (.f)
   - previously declared constants & variables