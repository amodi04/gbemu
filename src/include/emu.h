#pragma once

#include <common.h>

typedef struct {
    bool paused;
    bool running;
    u64 ticks;
} emu_context;

// Initializes and runs the emulator
int emu_run(int argc, char **argv);

// Global emulator context
emu_context *emu_get_context();

// Increments the emulator cycle count
void emu_cycles(int cpu_cycles);