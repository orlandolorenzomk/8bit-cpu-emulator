#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#define REG_COUNT 8

typedef struct {
    uint16_t PC;
    uint8_t R[REG_COUNT];   // R0..R7
    bool running;
    bool privileged;
} Cpu;


void cpu_init (Cpu *cpu, bool privileged);
void cpu_print (Cpu *cpu);

#endif
