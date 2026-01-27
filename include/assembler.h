#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdio.h>

/* ---------- public API ---------- */

/**
 * Assemble a program from the given input file into the given output file.
 * Returns the number of bytes written to output.
 */
int assemble(FILE *input, FILE *output, uint16_t *out_org);

#endif
