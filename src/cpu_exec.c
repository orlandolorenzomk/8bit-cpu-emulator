#include "cpu_exec.h"
#include "log.h"
#include "isa.h"
#include <stdint.h>

/*
 * Instruction formats:
 *
 * LOAD_IMM  : [opcode][reg][imm]
 * ADD/SUB   : [opcode][dst][src]
 * STORE     : [opcode][reg][hi][lo]
 * LOAD_MEM  : [opcode][reg][hi][lo]
 */

static void op_load_imm(Cpu *cpu, Ram *ram)
{
    uint8_t reg, imm;

    if (!ram_read(ram, cpu->PC++, &reg, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (reg) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &imm, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (imm) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (reg >= REG_COUNT)
    {
        log_write(LOG_ERROR, "LOAD_IMM invalid register R%d", reg);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "LOAD_IMM R%d <- 0x%02X", reg, imm);

    cpu->R[reg] = imm;
}

static void op_add(Cpu *cpu, Ram *ram)
{
    uint8_t dst, src;

    if (!ram_read(ram, cpu->PC++, &dst, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (dst) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &src, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (src) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (dst >= REG_COUNT || src >= REG_COUNT)
    {
        log_write(LOG_ERROR, "ADD invalid register dst=R%d src=R%d", dst, src);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "ADD R%d = R%d (0x%02X) + R%d (0x%02X)",
              dst, dst, cpu->R[dst], src, cpu->R[src]);

    cpu->R[dst] += cpu->R[src];
}

static void op_sub(Cpu *cpu, Ram *ram)
{
    uint8_t dst, src;

    if (!ram_read(ram, cpu->PC++, &dst, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (dst) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &src, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (src) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (dst >= REG_COUNT || src >= REG_COUNT)
    {
        log_write(LOG_ERROR, "SUB invalid register dst=R%d src=R%d", dst, src);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "SUB R%d = R%d (0x%02X) - R%d (0x%02X)",
              dst, dst, cpu->R[dst], src, cpu->R[src]);

    cpu->R[dst] -= cpu->R[src];
}

static void op_mlp(Cpu *cpu, Ram *ram)
{
    uint8_t dst, src;
    if (!ram_read(ram, cpu->PC++, &dst, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (dst) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &src, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (src) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (dst >= REG_COUNT || src >= REG_COUNT)
    {
        log_write(LOG_ERROR, "MLP invalid register dst=R%d src=R%d", dst, src);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "MLP R%d = R%d (0x%02X) + R%d (0x%02X)",
              dst, dst, cpu->R[dst], src, cpu->R[src]);
    cpu->R[dst] *= cpu->R[src];
}

static void op_div(Cpu *cpu, Ram *ram)
{
    uint8_t dst, src;
    if (!ram_read(ram, cpu->PC++, &dst, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (dst) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &src, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (src) at PC=0x%04X", cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (dst >= REG_COUNT || src >= REG_COUNT)
    {
        log_write(LOG_ERROR, "DIV invalid register dst=R%d src=R%d", dst, src);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "DIV R%d = R%d (0x%02X) + R%d (0x%02X)",
              dst, dst, cpu->R[dst], src, cpu->R[src]);
    cpu->R[dst] /= cpu->R[src];
}

static void op_store(Cpu *cpu, Ram *ram)
{
    uint8_t reg, hi, lo;

    if (!ram_read(ram, cpu->PC++, &reg, cpu->privileged) ||
        !ram_read(ram, cpu->PC++, &hi,  cpu->privileged) ||
        !ram_read(ram, cpu->PC++, &lo,  cpu->privileged))
    {
        log_write(LOG_ERROR, "STORE operand fetch failed");
        cpu->running = false;
        return;
    }

    if (reg >= REG_COUNT)
    {
        log_write(LOG_ERROR, "STORE invalid register R%d", reg);
        cpu->running = false;
        return;
    }

    uint16_t addr = (hi << 8) | lo;

    if (!ram_write(ram, addr, cpu->R[reg], cpu->privileged))
    {
        log_write(LOG_ERROR, "STORE write failed at 0x%04X", addr);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG,
              "STORE RAM[0x%04X] <- R%d (0x%02X)",
              addr, reg, cpu->R[reg]);
}

static void op_load_mem(Cpu *cpu, Ram *ram)
{
    uint8_t reg, hi, lo;

    if (!ram_read(ram, cpu->PC++, &reg, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (LOAD_MEM reg) at PC=0x%04X",
                  cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &hi, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (LOAD_MEM hi) at PC=0x%04X",
                  cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (!ram_read(ram, cpu->PC++, &lo, cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (LOAD_MEM lo) at PC=0x%04X",
                  cpu->PC - 1);
        cpu->running = false;
        return;
    }

    if (reg >= REG_COUNT)
    {
        log_write(LOG_ERROR, "LOAD_MEM invalid register R%d", reg);
        cpu->running = false;
        return;
    }

    uint16_t addr = (hi << 8) | lo;

    if (!ram_read(ram, addr, &cpu->R[reg], cpu->privileged))
    {
        log_write(LOG_ERROR, "RAM read failed (LOAD_MEM data) at addr=0x%04X",
                  addr);
        cpu->running = false;
        return;
    }

    log_write(LOG_DEBUG, "LOAD_MEM R%d <- RAM[0x%04X] (0x%02X)",
              reg, addr, cpu->R[reg]);
}

static void op_halt(Cpu *cpu, Ram *ram)
{
    (void)ram;

    log_write(LOG_INFO, "HALT instruction encountered");
    cpu->running = false;
}

static void op_invalid(Cpu *cpu, Ram *ram, uint8_t opcode)
{
    (void)ram;

    log_write(LOG_ERROR,
              "Invalid opcode 0x%02X at PC=0x%04X",
              opcode, cpu->PC - 1);

    cpu->running = false;
}

typedef void (*OpcodeHandler)(Cpu *, Ram *);

static OpcodeHandler handlers[256] =
{
    [OP_LOAD_IMM] = op_load_imm,
    [OP_ADD]      = op_add,
    [OP_SUB]      = op_sub,
    [OP_MLP]      = op_mlp,
    [OP_DIV]      = op_div,
    [OP_STORE]    = op_store,
    [OP_LOAD_MEM] = op_load_mem,
    [OP_HALT]     = op_halt,
};

void cpu_run(Cpu *cpu, Ram *ram, bool kernel)
{
    cpu->privileged = kernel;

    log_write(LOG_INFO, "CPU execution started at PC=0x%04X", cpu->PC);

    while (cpu->running)
    {
        uint8_t opcode;

        if (!ram_read(ram, cpu->PC++, &opcode, cpu->privileged))
        {
            log_write(LOG_ERROR, "Failed to fetch opcode at PC=0x%04X",
                      cpu->PC - 1);
            cpu->running = false;
            return;
        }

        log_write(LOG_TRACE, "Fetched opcode 0x%02X", opcode);

        OpcodeHandler handler = handlers[opcode];
        if (handler)
        {
            handler(cpu, ram);
        }
        else
        {
            op_invalid(cpu, ram, opcode);
        }
    }

    log_write(LOG_INFO, "CPU execution stopped");
}
