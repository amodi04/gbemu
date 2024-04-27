#pragma once

#include <common.h>

// Returns the value at the given address in WRAM
u8 wram_read(u16 address);

// Writes the given value to the given address in WRAM
void wram_write(u16 address, u8 value);

// Returns the value at the given address in HRAM
u8 hram_read(u16 address);

// Writes the given value to the given address in HRAM
void hram_write(u16 address, u8 value);