#pragma once

#include <common.h>

// Defines the Gameboy cartridge interface and header structure
// https://gbdev.io/pandocs/The_Cartridge_Header.html
typedef struct {
    u8 entry[4];
    u8 logo[0x30];

    char title[16];
    u16 new_lic_code;
    u8 sgb_flag;
    u8 type;
    u8 rom_size;
    u8 ram_size;
    u8 dest_code;
    u8 lic_code;
    u8 version;
    u8 checksum;
    u16 global_checksum;
} rom_header;

// True if the cartridge is loaded successfully
bool cart_load(char *cart);

// Reads a byte from the cartridge at the given address
u8 cart_read(u16 address);

// Writes a byte to the cartridge at the given address
void cart_write(u16 address, u8 value);