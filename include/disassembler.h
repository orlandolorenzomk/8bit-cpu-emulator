#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>

void disassemble_memory(const uint8_t *memory, uint16_t start_addr, uint16_t end_addr);

#endif
