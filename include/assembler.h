#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* ---------- limits ---------- */

#define ASM_MAX_LINE_LEN 256
#define ASM_MAX_LABELS 1024
#define ASM_MAX_TOKENS 8

/* ---------- label table ---------- */

typedef struct
{
    char name[64];
    uint16_t address;
} AsmLabel;

/* ---------- assembler context ---------- */

typedef struct
{
    AsmLabel labels[ASM_MAX_LABELS];
    size_t label_count;

    uint16_t origin; /* current .org */
    uint16_t pc;     /* program counter during assembly */
} Assembler;

/* ---------- API ---------- */

/* lifecycle */
void assembler_init(Assembler *as);
void assembler_reset(Assembler *as);

/* main entry point */
bool assembler_assemble_file(
    Assembler *as,
    const char *input_path,
    uint8_t *output,
    size_t output_size,
    size_t *out_len);

/* internal passes */
bool assembler_first_pass(Assembler *as, FILE *in);
bool assembler_second_pass(
    Assembler *as,
    FILE *in,
    uint8_t *output,
    size_t output_size,
    size_t *out_len);

/* label handling */
bool assembler_add_label(Assembler *as, const char *name, uint16_t addr);
bool assembler_find_label(
    const Assembler *as,
    const char *name,
    uint16_t *out_addr);

/* utilities */
void assembler_strip_comment(char *line);
size_t assembler_tokenize(char *line, char *tokens[], size_t max_tokens);

/* public API */
int assemble(FILE *input, FILE *output);

#endif
