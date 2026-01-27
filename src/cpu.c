#include "cpu.h"
#include "log.h"

void cpu_init(Cpu *cpu, bool privileged)
{
    log_write(LOG_INFO, "CPU initialization started");

    log_write(LOG_INFO, "There are %d [R%d - R%d] registers available for CPU instructions", REG_COUNT, (REG_COUNT - REG_COUNT), REG_COUNT - 1);
    for (int i = 0; i < REG_COUNT; i++)
    {
        cpu->R[i] = 0;
        log_write(LOG_DEBUG, "Register R%d initialized to 0", i);
    }

    cpu->PC = 0;
    log_write(LOG_DEBUG, "Program Counter set to 0");

    cpu->running = true;
    log_write(LOG_DEBUG, "CPU running flag set to true");

    cpu->privileged = privileged;
    log_write(LOG_DEBUG, "CPU privileged mode set to %s",
              privileged ? "true" : "false");

    log_write(LOG_INFO, "CPU initialization completed successfully");
}

void cpu_print(Cpu *cpu)
{
    log_write(LOG_INFO, "Printing CPU information");

    log_write(
        LOG_INFO,
        "There are %d registers available (R0 - R%d)",
        REG_COUNT,
        REG_COUNT - 1
    );

    for (int i = 0; i < REG_COUNT; i++)
    {
        log_write(
            LOG_DEBUG,
            "Register R%d value: %u",
            i,
            cpu->R[i]
        );
    }

    log_write(
        LOG_INFO,
        "Program Counter pointing at 0x%04X",
        cpu->PC
    );

    log_write(
        LOG_INFO,
        "CPU state: %s",
        cpu->running ? "running" : "halted"
    );

    log_write(
        LOG_INFO,
        "Privilege mode: %s",
        cpu->privileged ? "privileged" : "user"
    );
}
