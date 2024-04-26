#pragma once

#include <common.h>

// Reads a byte from the bus at the given address
u8 bus_read(u16 address);

// Writes a byte to the bus at the given address
void bus_write(u16 address, u8 value);

// Reads a 16-bit value from the bus at the given address
u16 bus_read16(u16 address);

// Writes a 16-bit value to the bus at the given address
void bus_write16(u16 address, u16 value);