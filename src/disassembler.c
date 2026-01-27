#include "disassembler.h"
#include "isa.h"
#include "log.h"
#include <stdio.h>

static uint8_t read8(const uint8_t *memory, uint16_t addr)
{
    return memory[addr];
}

static uint16_t read16(const uint8_t *memory, uint16_t addr)
{
    uint16_t hi = read8(memory, addr);
    uint16_t lo = read8(memory, addr + 1);
    return (hi << 8) | lo;
}

static void disasm_load_imm(const uint8_t *memory, uint16_t pc, uint16_t *next_pc)
{
    uint8_t reg = read8(memory, pc + 1);
    uint8_t imm = read8(memory, pc + 2);
    log_write(LOG_INFO, "[DISASSEMBLER] 0x%04X: LOAD_IMM R%u, #%u", pc, reg, imm);
    *next_pc = pc + 3;
}

static void disasm_two_reg(const uint8_t *memory, uint16_t pc, uint16_t *next_pc, const char *name)
{
    uint8_t dst = read8(memory, pc + 1);
    uint8_t src = read8(memory, pc + 2);
    log_write(LOG_INFO, "[DISASSEMBLER] 0x%04X: %s R%u, R%u", pc, name, dst, src);
    *next_pc = pc + 3;
}

static void disasm_mem_op(const uint8_t *memory, uint16_t pc, uint16_t *next_pc, const char *name)
{
    uint8_t reg = read8(memory, pc + 1);
    uint16_t addr = read16(memory, pc + 2);
    log_write(LOG_INFO, "[DISASSEMBLER] 0x%04X: %s R%u, 0x%04X", pc, name, reg, addr);
    *next_pc = pc + 4;
}

static void disasm_halt(uint16_t pc, uint16_t *next_pc)
{
    log_write(LOG_INFO, "[DISASSEMBLER] 0x%04X: HALT", pc);
    *next_pc = pc + 1;
}

static void disasm_unknown(const uint8_t *memory, uint16_t pc, uint16_t *next_pc)
{
    uint8_t opcode = read8(memory, pc);
    log_write(LOG_INFO, "[DISASSEMBLER] 0x%04X: DB 0x%02X", pc, opcode);
    *next_pc = pc + 1;
}

void disassemble_memory(const uint8_t *memory, uint16_t start_addr, uint16_t end_addr)
{
    uint16_t pc = start_addr;

    while (pc <= end_addr)
    {
        uint8_t opcode = read8(memory, pc);
        uint16_t next_pc = pc;

        switch (opcode)
        {
        case OP_LOAD_IMM:
            disasm_load_imm(memory, pc, &next_pc);
            break;

        case OP_ADD:
            disasm_two_reg(memory, pc, &next_pc, "ADD");
            break;

        case OP_SUB:
            disasm_two_reg(memory, pc, &next_pc, "SUB");
            break;

        case OP_MLP:
            disasm_two_reg(memory, pc, &next_pc, "MLP");
            break;

        case OP_DIV:
            disasm_two_reg(memory, pc, &next_pc, "DIV");
            break;

        case OP_STORE:
            disasm_mem_op(memory, pc, &next_pc, "STORE");
            break;

        case OP_LOAD_MEM:
            disasm_mem_op(memory, pc, &next_pc, "LOAD_MEM");
            break;

        case OP_HALT:
            disasm_halt(pc, &next_pc);
            break;

        default:
            disasm_unknown(memory, pc, &next_pc);
            break;
        }

        pc = next_pc;
    }
}
