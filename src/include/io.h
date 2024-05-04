#pragma once

#include <common.h>

// Reads a byte from the given IO address
u8 io_read(u16 addr);

// Writes a byte to the given IO address
void io_write(u16 addr, u8 value);