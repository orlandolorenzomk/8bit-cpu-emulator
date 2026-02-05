# 8 Bit CPU Emulator
-- Lorenzo Orlando

## Overview
This project is a small 8-bit CPU emulator written in C. It includes a basic assembler, a RAM module with privilege checking, and a CPU that executes a simple instruction set.

## Features
- Assembler for a custom assembly language
- 64KB RAM with user and privileged memory regions
Simple 8-bit CPU with 8 registers and a small instruction set
- Logging system with multiple log levels
- Two-pass assembler supporting labels and .org
- Privilege mode enforced at memory access level

## Instruction Set

The emulator supports the following instructions:

| Opcode | Instruction | Format                  | Description                               |
| ------ | ----------- | ----------------------- | ----------------------------------------- |
| `1`    | `LOAD_IMM`  | `[opcode][reg][imm]`    | Load immediate value into register        |
| `2`    | `SUB`       | `[opcode][dst][src]`    | Subtract source register from destination |
| `3`    | `ADD`       | `[opcode][dst][src]`    | Add source register to destination        |
| `4`    | `STORE`     | `[opcode][reg][hi][lo]` | Store register value into memory          |
| `5`    | `LOAD_MEM`  | `[opcode][reg][hi][lo]` | Load memory value into register           |
| `6`    | `MLP`       | `[opcode][dst][src]`    | Multiply destination by source            |
| `7`    | `DIV`       | `[opcode][dst][src]`    | Divide destination by source              |
| `255`  | `HALT`      | `[opcode]`              | Stop execution                            |

## Memory

RAM is 64KB (65536 bytes). The first 8192 bytes (0x0000–0x1FFF) are privileged.

Privileged memory range: 0x0000 to 0x1FFF

User mode can only access memory from 0x2000 onwards

Privilege is enforced in ram_read and ram_write

## Assembler

The assembler is a two-pass assembler.

### Pass 1
 - Parses labels
 - Records addresses
 - Handles -org

### Pass 2
- Emits opcodes and operands 
- Resolves label addresses

The assembler outputs a binary file ```out.bin``` and returns the origin adddress.

## CPU

The CPU has:
- Program Counter (`PC`)
- 8 registers (`R0-R7`)
- Running flag
- Privilege flag

CPU execution is handled by the function ```cpu_run``` inside ```cpu.c``` which fetches opcodes from RAM and executes them.

## How to Run

Compile and run:
- ```make```
- ```./cpu-emulator /path/to/program.asm```

## Example Assembly Program
```
.org 0x2001

start:
    LOAD_IMM R0, #5
    LOAD_IMM R1, #3
    ADD      R0, R1
    STORE    R0, 0x2002
    LOAD_MEM R2, 0x2002
    HALT
```

## Logging
Logging is implemented in `log.c` with the following levels:
- `INFO`
- `DEBUG`
- `WARN`
- `TRACE`
- `ERROR`
- `UNAUTHORIZED`

Log outputs includes a timestamp and log level.

## Notes
- The emulator currently loads the assembled binary into RAM at the origin address specified by .org
- Running in user mode requires the origin to be in user space (>= 0x2000)
- CPU execution can be run in privileged or user mode using the `kernel` flag in `cpu_run`

## File Structure

```
include/
  assembler.h
  cpu.h
  cpu_exec.h
  isa.h
  log.h
  ram.h

src/
  assembler.c
  cpu.c
  cpu_exec.c
  log.c
  main.c
  ram.c

```
