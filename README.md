<div align="center">

# 🖥️ Custom Processor Design & Simulation

**A cycle-by-cycle simulator, written in C, of a 32-bit processor with its own 12-instruction ISA, a Von Neumann memory and a 5-stage pipeline. It reads an assembly program, encodes it into machine code in memory, and runs it while printing every stage of every clock cycle.**

[![CI](https://github.com/DanEhab/Custom_Processor_Design_and_Simulation_Project/actions/workflows/ci.yml/badge.svg)](https://github.com/DanEhab/Custom_Processor_Design_and_Simulation_Project/actions/workflows/ci.yml)
![Language](https://img.shields.io/badge/Language-C-00599c?logo=c&logoColor=white)
![Course](https://img.shields.io/badge/Course-Computer%20System%20Architecture-1f6feb)
![Semester](https://img.shields.io/badge/Semester-Spring%202025-8957e5)
![Platforms](https://img.shields.io/badge/Runs%20on-Linux%20%C2%B7%20macOS%20%C2%B7%20Windows-555)
![Grade](https://img.shields.io/badge/Course%20grade-A%2B-success)

</div>

---

## The processor

| | |
|---|---|
| **Memory** | Von Neumann: one memory of 2048 words × 32 bits, word addressable. Addresses 0–1023 hold the program and 1024–2047 hold data. |
| **Registers** | R0 (hard-wired to 0), 31 general-purpose registers R1–R31, and the PC. All are 32 bits. |
| **Instructions** | 32 bits each, in three formats: |

```
R-format   | opcode (4) | R1 (5) | R2 (5) | R3 (5) | SHAMT (13) |
I-format   | opcode (4) | R1 (5) | R2 (5) |   IMMEDIATE (18)    |
J-format   | opcode (4) |          ADDRESS (28)                  |
```

| Opcode | Instruction | Format | Operation |
|:---:|---|:---:|---|
| 0 | `ADD R1 R2 R3` | R | R1 = R2 + R3 |
| 1 | `SUB R1 R2 R3` | R | R1 = R2 − R3 |
| 2 | `MUL R1 R2 R3` | R | R1 = R2 × R3 |
| 3 | `MOVI R1 IMM` | I | R1 = IMM |
| 4 | `JEQ R1 R2 IMM` | I | if R1 == R2: PC = PC + 1 + IMM |
| 5 | `AND R1 R2 R3` | R | R1 = R2 & R3 |
| 6 | `XORI R1 R2 IMM` | I | R1 = R2 ⊕ IMM |
| 7 | `JMP ADDRESS` | J | PC = PC[31:28] ‖ ADDRESS |
| 8 | `LSL R1 R2 SHAMT` | R | R1 = R2 << SHAMT |
| 9 | `LSR R1 R2 SHAMT` | R | R1 = R2 >>> SHAMT (logical) |
| 10 | `MOVR R1 R2 IMM` | I | R1 = MEM[R2 + IMM] |
| 11 | `MOVM R1 R2 IMM` | I | MEM[R2 + IMM] = R1 |

Immediates are signed 18-bit values (−131,072 to 131,071), so `JEQ` can also branch backwards.

## The pipeline

Every instruction goes through Fetch, Decode, Execute, Memory and Write Back. Fetch and Memory share the one memory, so they never run in the same cycle: odd cycles run IF and WB, even cycles run ID, EX and MEM. An instruction spends 2 cycles in ID, 2 in EX, 1 in MEM and 1 in WB, so up to 4 instructions are in flight and a program of *n* instructions takes **7 + (n − 1) × 2** cycles. This is the start of a run, as the simulator prints it:

```
Cycle 1   Pipeline | IF: inst 0   ID: -        EX: -        MEM: -        WB: -
Cycle 2   Pipeline | IF: -        ID: inst 0   EX: -        MEM: -        WB: -
Cycle 3   Pipeline | IF: inst 1   ID: inst 0   EX: -        MEM: -        WB: -
Cycle 4   Pipeline | IF: -        ID: inst 1   EX: inst 0   MEM: -        WB: -
Cycle 5   Pipeline | IF: inst 2   ID: inst 1   EX: inst 0   MEM: -        WB: -
Cycle 6   Pipeline | IF: -        ID: inst 2   EX: inst 1   MEM: inst 0   WB: -
Cycle 7   Pipeline | IF: inst 3   ID: inst 2   EX: inst 1   MEM: -        WB: inst 0
```

- **Jumps** are resolved in Execute. When `JEQ` or `JMP` is taken, the PC is updated and the one instruction fetched behind it is flushed.
- **Data hazards** are not handled, as in the project description: there is no forwarding and no stalling. A result can be read by the second instruction after the one that writes it, so the programs leave one instruction between them (the [no-forwarding test](tests/programs/12_no_forwarding.asm) shows what happens otherwise).

Each cycle also prints what each stage does with its inputs, every register and memory word that changes, and at the end the content of all registers and the whole memory. Here is the cycle where a branch is taken, from [`05_branch_taken.asm`](tests/programs/05_branch_taken.asm):

```
Cycle Num: 10
I am Currently in the memory phase 📝 (instruction: 00110100100000000000000000000000)
This instruction doesn't want to access the RAM 🤷‍♂️
I am Currently executing ⚙️ (instruction: 01000000100010000000000000000001)
I am currently checking if the content of Regs1 is equal to the content of Regs2, jump to immediate address
Reg1: 3
Reg2: 3
Immediate Value: 1
Now the ALU is: 5 (PC + 1 + IMM = 3 + 1 + 1)
PC is updated to ---> 5
Flushing inst 4, which was fetched before the jump was taken 🚿
Pipeline | IF: -   ID: -   EX: inst 3   MEM: inst 2   WB: -
```

## How it works

1. **Assembling** ([`translate.c`](translate.c)): reads the program, checks every instruction, and encodes it as a 32-bit binary word stored at addresses 0, 1, 2, … of the memory. Nothing else about the program is kept, so the processor only ever sees machine code.
2. **Decoding** ([`processor.c`](processor.c)): splits the instruction register into the opcode and the fields of its format, and sign-extends the immediate.
3. **Pipeline**: the memory and every register are arrays of 32 bits. Queues act as the pipeline registers between the stages and carry a `Data` record (opcode, registers, immediate, ALU result, the instruction's address) from one stage to the next.

## Run it

You need a C compiler (gcc or clang).

```console
$ make                                   # or: gcc translate.c -o processor
$ ./processor instructions.txt           # 500 ms per cycle, so you can watch it
$ ./processor tests/programs/08_loop_sum.asm --delay 0
```

On Windows: `gcc translate.c -o processor.exe` and `processor.exe instructions.txt`.

Programs are plain text with one instruction per line. Comments start with `#` or `;`, and lower case, tabs and commas are fine:

```asm
MOVI R1 0          # i
MOVI R2 4          # n
MOVI R4 1
MOVI R3 0          # sum
ADD R1 R1 R4       # loop: i = i + 1
MOVI R9 0          # gap (no forwarding)
ADD R3 R3 R1       # sum = sum + i
JEQ R1 R2 1        # if i == n, leave the loop
JEQ R0 R0 -5       # always taken: back to the loop
MOVM R3 R0 1024    # MEM[1024] = 1 + 2 + 3 + 4 = 10
```

## Tests

[`tests/run_tests.py`](tests/run_tests.py) builds the simulator and runs **13 programs** that cover every instruction, taken and untaken branches, a backwards loop, memory loads and stores, R0, the cycle formula and the sample program in [`instructions.txt`](instructions.txt). For each one it checks the cycle count, the PC, every register and the whole data segment against values worked out by hand. It also checks that **5 invalid programs** are rejected with a clear error. GitHub Actions runs it on Linux, macOS and Windows.

```console
$ python3 tests/run_tests.py
PASS  programs/01_straight_line.asm (19 cycles)
PASS  programs/02_arithmetic.asm (19 cycles)
...
PASS  programs/08_loop_sum.asm (61 cycles)
...
All tests passed.
```

## After the submission

The first 20 commits (May to June 2025) are the project as it was built and submitted. Running it again later showed some problems, which the later commits fix:

- When a program didn't end with a jump, the last instruction was decoded and executed again and again until a limit of about 1,000 extra cycles, so a final `ADD R1 R1 R2` ran over 500 times.
- After a jump, the instructions that followed went through Decode, Execute and Memory in a single cycle, so the timing no longer matched the pipeline.
- `JEQ` could not branch backwards, and two test values (R1 = R2 = 5) were still set at the start of every program.
- It only built on Windows. It now builds on Linux and macOS too, and the delay per cycle is an option.
- The assembler read uninitialised memory when a line was malformed (the sample program has a non-breaking space in one line) and wrote one byte past a buffer. It now reports clear errors with the line number.

The project description and the extra remarks from the course are in [`docs/`](docs).

---

## Author & course

Designed and written entirely by **Daniel Ehab**, and submitted as the project of Team 1 for **Computer System Architecture (CSEN 601 / CSEN B501)** at the German International University in Berlin, Spring 2025, during a semester abroad from the German University in Cairo.
