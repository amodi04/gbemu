#pragma once

#include <common.h>

// Defines the Gameboy CPU (LR35902) stack

// Pushes a byte onto the stack
void stack_push(u8 data);

// Pushes a 16-bit word onto the stack
void stack_push16(u16 data);

// Pops a byte from the stack
u8 stack_pop();

// Pops a 16-bit word from the stack
u16 stack_pop16();