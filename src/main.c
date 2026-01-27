#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include "ram.h"
#include "cpu.h"
#include "cpu_exec.h"
#include "log.h"
#include "assembler.h"

static long long time_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <asm_file>\n", argv[0]);
        return 1;
    }

    const char *asm_path = argv[1];
    const char *bin_path = "out.bin";

    long long start = time_now_ms();

    FILE *in = fopen(asm_path, "r");
    if (!in)
    {
        log_write(LOG_ERROR, "Error while opening %s", asm_path);
        return 1;
    }

    FILE *out = fopen(bin_path, "wb");
    if (!out)
    {
        log_write(LOG_ERROR, "Error while opening %s", bin_path);
        fclose(in);
        return 1;
    }

    uint16_t org = 0;
    int size = assemble(in, out, &org);
    fclose(in);
    fclose(out);

    log_write(LOG_INFO, "Assembled %d bytes -> %s", size, bin_path);

    Cpu cpu;
    Ram ram;

    bool privileged = false;
    cpu_init(&cpu, privileged);
    ram_init(&ram);

    FILE *bin = fopen(bin_path, "rb");
    if (!bin)
    {
        log_write(LOG_ERROR, "Error while opening %s", bin_path);
        return 1;
    }

    uint8_t buffer[RAM_SIZE];
    size_t read = fread(buffer, 1, sizeof(buffer), bin);
    fclose(bin);

    uint16_t load_addr = org;

    for (size_t i = 0; i < read; i++)
    {
        if (!ram_write(&ram, load_addr + i, buffer[i], privileged))
        {
            log_write(LOG_ERROR, "Failed to write program to RAM at 0x%04X", load_addr + i);
            return 1;
        }
    }

    cpu.PC = org;
    cpu.running = true;

    cpu_run(&cpu, &ram, true);

    uint8_t result = 0;
    ram_read(&ram, 0x2000, &result, privileged);

    log_write(LOG_INFO, "Result: %u", result);

    cpu_print(&cpu);

    long long end = time_now_ms();
    log_write(LOG_INFO, "Elapsed time: %lld ms (%.3f s)", end - start, (end - start) / 1000.0);

    return 0;
}
