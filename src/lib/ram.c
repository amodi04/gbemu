#include <ram.h>

typedef struct {
    u8 wram[0x2000];
    u8 hram[0x80];
} ram_context;

static ram_context ctx;

u8 wram_read(u16 address) {
    // Offset WRAM address (0xC000 - 0xDFFF)
    address -= 0xC000;

    // WRAM is 8KB
    if (address >= 0x2000) {
        printf("Invalid WRAM read addr %08X\n", address + 0xC000);
        exit(-1);
    }

    return ctx.wram[address];
}

void wram_write(u16 address, u8 value) {
    // Offset WRAM address (0xC000 - 0xDFFF)
    address -= 0xC000;

    // WRAM is 8KB
    if (address >= 0x2000) {
        printf("Invalid WRAM write addr %08X\n", address + 0xC000);
        exit(-1);
    }

    ctx.wram[address] = value;
}

u8 hram_read(u16 address) {
    // Offset HRAM address (0xFF80 - 0xFFFE)
    address -= 0xFF80;

    // HRAM is 128B
    if (address >= 0x80) {
        printf("Invalid HRAM read addr %08X\n", address + 0xFF80);
        exit(-1);
    }

    return ctx.hram[address];
}

void hram_write(u16 address, u8 value) {
    // Offset HRAM address (0xFF80 - 0xFFFE)
    address -= 0xFF80;

    // HRAM is 128B
    if (address >= 0x80) {
        printf("Invalid HRAM write addr %08X\n", address + 0xFF80);
        exit(-1);
    }

    ctx.hram[address] = value;
}