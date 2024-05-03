#pragma once

#include <cpu.h>

typedef enum {
    INT_VBLANK   = 0b00001,
    INT_LCD_STAT = 0b00010,
    INT_TIMER    = 0b00100,
    INT_SERIAL   = 0b01000,
    INT_JOYPAD   = 0b10000
} interrupt_type;

// Requests an interrupt
void cpu_request_interrupt(interrupt_type type);

// Handles all interrupts
void cpu_handle_interrupts(cpu_context *ctx);