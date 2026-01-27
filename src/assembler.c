#include "assembler.h"
#include "isa.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "cpu.h"

#define MAX_LINE 256
#define MAX_OUTPUT 65536

typedef struct
{
    char name[64];
    uint16_t addr;
} Label;

typedef struct
{
    const char *mnemonic;
    uint8_t opcode;
    uint8_t size;
} InstrDef;

/* ================= instruction table ================= */

static const InstrDef instr_table[] =
    {
        {"LOAD_IMM", OP_LOAD_IMM, 3},
        {"ADD", OP_ADD, 3},
        {"SUB", OP_SUB, 3},
        {"MLP", OP_MLP, 3},
        {"DIV", OP_DIV, 3},
        {"STORE", OP_STORE, 4},
        {"LOAD_MEM", OP_LOAD_MEM, 4},
        {"HALT", OP_HALT, 1},
};

#define INSTR_COUNT (sizeof(instr_table) / sizeof(instr_table[0]))

/* ================= state ================= */

static Label *labels = NULL;
static size_t label_count = 0;
static size_t label_cap = 0;

static uint8_t output_buf[MAX_OUTPUT];
static size_t out_pos = 0;
static uint16_t pc = 0;

/* ================= utilities ================= */

static void fatal(const char *msg, int line)
{
    fprintf(stderr, "Assembler error (line %d): %s\n", line, msg);
    exit(1);
}

static void fatal_fmt(const char *fmt, const char *instr, int line_no)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), fmt, instr);
    fatal(buffer, line_no);
}

static char *trim(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    char *e = s + strlen(s) - 1;
    while (e >= s && isspace((unsigned char)*e))
        *e-- = '\0';
    return s;
}

static void ensure_label_cap(void)
{
    if (label_count < label_cap)
        return;
    label_cap = label_cap ? label_cap * 2 : 16;
    labels = realloc(labels, label_cap * sizeof(Label));
    if (!labels)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
}

static int find_label(const char *name)
{
    for (size_t i = 0; i < label_count; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
            return (int)i;
    }
    return -1;
}

static void add_label(const char *name, uint16_t addr, int line)
{
    if (find_label(name) >= 0)
        fatal("Duplicate label", line);

    ensure_label_cap();
    strncpy(labels[label_count].name, name, sizeof(labels[label_count].name));
    labels[label_count].addr = addr;
    label_count++;
}

static uint16_t parse_number(const char *s)
{
    if (strncmp(s, "0x", 2) == 0)
        return (uint16_t)strtol(s + 2, NULL, 16);
    return (uint16_t)strtol(s, NULL, 10);
}

static uint8_t reg_num(const char *s)
{
    if (strlen(s) != 2 || s[0] != 'R')
        return -1;
    int r = s[1] - '0';
    return (r >= 0 && r < REG_COUNT) ? r : -1;
}

static const InstrDef *find_instr(const char *mnem)
{
    for (size_t i = 0; i < INSTR_COUNT; i++)
    {
        if (strcmp(instr_table[i].mnemonic, mnem) == 0)
            return &instr_table[i];
    }
    return NULL;
}

static void emit8(uint8_t v)
{
    output_buf[out_pos++] = v;
}

static void emit16(uint16_t v)
{
    emit8((v >> 8) & 0xFF);
    emit8(v & 0xFF);
}

static inline char *next_token(void)
{
    return strtok(NULL, " ,");
}

static inline bool missing_operands(char *a, char *b)
{
    return a == NULL || b == NULL;
}

static inline bool missing_operand(char *operand)
{
    return operand == NULL;
}

static inline bool invalid_register(int reg)
{
    return reg < 0 || reg >= REG_COUNT;
}

/* ======= helper functions for pass 2 ======*/

static void emit_reg_imm_instruction(
    uint8_t opcode,
    const char *instruction_name,
    int line_no)
{
    char *register_token = next_token();
    char *immediate_token = next_token();

    if (register_token == NULL)
        fatal_fmt("[%s] Missing register operand", instruction_name, line_no);

    if (immediate_token == NULL)
        fatal_fmt("[%s] Missing immediate operand", instruction_name, line_no);

    int reg = reg_num(register_token);
    if (reg < 0)
        fatal_fmt("[%s] Invalid register", instruction_name, line_no);

    if (*immediate_token == '#')
        immediate_token++;

    emit8(opcode);
    emit8((uint8_t)reg);
    emit8((uint8_t)parse_number(immediate_token));
}

static void emit_two_register_instruction(
    uint8_t opcode,
    const char *instruction_name,
    int line_number)
{
    char *destination_operand = next_token();
    char *source_operand = next_token();

    if (destination_operand == NULL)
    {
        fatal_fmt("[%s] missing [DST] operand", instruction_name, line_number);
    }

    if (source_operand == NULL)
    {
        fatal_fmt("[%s] missing [SRC] operand", instruction_name, line_number);
    }

    int destination_register = reg_num(destination_operand);
    int source_register = reg_num(source_operand);

    if (destination_register < 0)
    {
        fatal_fmt("[%s] Invalid [DST] register", instruction_name, line_number);
    }

    if (source_register < 0)
    {
        fatal_fmt("[%s] Invalid [SRC] register", instruction_name, line_number);
    }

    emit8(opcode);
    emit8((uint8_t)destination_register);
    emit8((uint8_t)source_register);
}

static void emit_reg_addr_instruction(
    uint8_t opcode,
    int line_no)
{
    char *register_token = next_token();
    char *address_token = next_token();

    if (register_token == NULL || address_token == NULL)
        fatal("Missing operands", line_no);

    int reg = reg_num(register_token);
    if (reg < 0)
        fatal("Invalid register", line_no);

    uint16_t address;

    if (isdigit((unsigned char)address_token[0]))
    {
        address = parse_number(address_token);
    }
    else
    {
        int label_index = find_label(address_token);
        if (label_index < 0)
            fatal("Unknown label", line_no);

        address = labels[label_index].addr;
    }

    emit8(opcode);
    emit8((uint8_t)reg);
    emit16(address);
}

/* ================= pass 1 ================= */

static void pass1(FILE *f)
{
    char line[MAX_LINE];
    int line_no = 0;

    while (fgets(line, sizeof(line), f))
    {
        line_no++;
        char *p = trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        char *colon = strchr(p, ':');
        if (colon)
        {
            *colon = '\0';
            add_label(trim(p), pc, line_no);
            p = trim(colon + 1);
            if (*p == '\0')
                continue;
        }

        if (strncmp(p, ".org", 4) == 0)
        {
            pc = parse_number(trim(p + 4));
            continue;
        }

        char *mn = strtok(p, " ,");
        const InstrDef *ins = find_instr(mn);
        if (!ins)
            fatal("Unknown instruction", line_no);

        pc += ins->size;
    }
}

/* ================= pass 2 ================= */

static void pass2(FILE *f)
{
    rewind(f);
    pc = 0;
    out_pos = 0;

    char line[MAX_LINE];
    int line_no = 0;

    while (fgets(line, sizeof(line), f))
    {
        line_no++;
        char *p = trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        char *colon = strchr(p, ':');
        if (colon)
        {
            p = trim(colon + 1);
            if (*p == '\0')
                continue;
        }

        if (strncmp(p, ".org", 4) == 0)
        {
            pc = parse_number(trim(p + 4));
            continue;
        }

        char *mn = strtok(p, " ,");
        const InstrDef *ins = find_instr(mn);
        if (!ins)
            fatal("Unknown instruction", line_no);

        switch (ins->opcode)
        {

        case OP_LOAD_IMM:
            emit_reg_imm_instruction(ins->opcode, "LOAD", line_no);
            break;

        case OP_ADD:
            emit_two_register_instruction(ins->opcode, "ADD", line_no);
            break;

        case OP_SUB:
            emit_two_register_instruction(ins->opcode, "SUB", line_no);
            break;

        case OP_MLP:
            emit_two_register_instruction(ins->opcode, "MLP", line_no);
            break;

        case OP_DIV:
            emit_two_register_instruction(ins->opcode, "DIV", line_no);
            break;

        case OP_STORE:
        case OP_LOAD_MEM:
            emit_reg_addr_instruction(ins->opcode, line_no);
            break;

        case OP_HALT:
            emit8(OP_HALT);
            break;

        default:
            fatal("Unhandled opcode", line_no);
        }

        pc += ins->size;
    }
}

/* ================= public API ================= */

int assemble(FILE *input, FILE *output)
{
    pass1(input);
    pass2(input);

    fwrite(output_buf, 1, out_pos, output);
    return (int)out_pos;
}
