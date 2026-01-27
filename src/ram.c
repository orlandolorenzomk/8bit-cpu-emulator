#include "ram.h"
#include "log.h"
#include "inttypes.h"
#include "string.h"

static bool is_address_valid(uint32_t address, bool privileged);

void ram_init(Ram *ram)
{
    log_write(LOG_INFO, "Setting all memory cells to 0");
    memset(ram->memory_cells, 0, RAM_SIZE);
    log_write(LOG_INFO, "RAM initialized correctly");
}

bool ram_read(Ram *ram, uint32_t address, uint8_t *output, bool privileged)
{
    if (!is_address_valid(address, privileged))
        return false;

    *output = ram->memory_cells[address];

    log_write(LOG_DEBUG,
              "RAM READ  address=0x%04" PRIX32 " value=0x%02X",
              address, *output);

    return true;
}

bool ram_write(Ram *ram, uint32_t address, uint8_t value, bool privileged)
{
    if (!is_address_valid(address, privileged))
        return false;

    ram->memory_cells[address] = value;

    log_write(LOG_DEBUG,
              "RAM WRITE addr=0x%04" PRIX32 " value=0x%02X",
              address, value);

    return true;
}

static bool is_address_valid(uint32_t address, bool privileged)
{
    if (address >= RAM_SIZE)
    {
        log_write(LOG_DEBUG,
                  "Given address %" PRIu32 " is out of bounds (RAM_SIZE=%" PRIu32 ")",
                  address, (uint32_t)RAM_SIZE);
        return false;
    }

    if (address <= RAM_PRIVILEGED_MODE_END &&
        !privileged)
    {
        log_write(LOG_UNAUTHORIZED,
                  "Trying to read or write in an unauthorized memory location.");
        return false;
    }

    return true;
}
