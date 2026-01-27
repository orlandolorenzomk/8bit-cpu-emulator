#pragma once
#include <stdint.h>

typedef enum {
    OP_LOAD_IMM = 1,   // reg = immediate value
    OP_SUB      = 2,   // dst = dst - src
    OP_ADD      = 3,   // dst = dst + src
    OP_STORE    = 4,   // memory[addr] = reg
    OP_LOAD_MEM = 5,   // reg = memory[addr]
    OP_MLP      = 6,   // dst = dst * src
    OP_DIV      = 7,   // dst = dst / src
    OP_HALT     = 255  // stop CPU execution
} Opcode;
