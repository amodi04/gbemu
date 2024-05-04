#pragma once

#include <common.h>

typedef struct {
    u16 div; // Divider register
    u8 tima; // Timer counter
    u8 tma; // Timer modulo
    u8 tac; // Timer control
} timer_context;

// Initializes the timer
void timer_init();

// Updates the timer
void timer_tick();

// Writes to the timer
void timer_write(u16 address, u8 value);

// Reads from the timer
u8 timer_read(u16 address);

// Gets the timer context
timer_context *timer_get_context();