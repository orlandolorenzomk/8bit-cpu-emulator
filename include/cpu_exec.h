#ifndef CPU_EXEC_H
#define CPU_EXEC_H

#include "cpu.h"
#include "ram.h"

void cpu_run(Cpu *cpu, Ram *ram, bool kernel);

#endif
