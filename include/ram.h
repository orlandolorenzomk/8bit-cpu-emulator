#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE 65536 // 64KB Ram

#define RAM_PRIVILEGED_MODE_START 0
#define RAM_PRIVILEGED_MODE_END   8192

// Defining 65536 (64KB) memory cells. Each 1 byte
typedef struct {
    uint8_t memory_cells[RAM_SIZE];
} Ram;

void ram_init(Ram *ram);
bool ram_read(Ram *ram, uint32_t address, uint8_t *output, bool privileged);
bool ram_write(Ram *ram, uint32_t address, uint8_t value, bool privileged);

#endif
