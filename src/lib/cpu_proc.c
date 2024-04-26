#include <cpu.h>
#include <emu.h>

// Processes CPU instructions

// Invalid instruction
static void proc_none(cpu_context *ctx) {
    printf("INVALID INSTRUCTION\n");
    exit(-7);
}

// No operation
static void proc_nop(cpu_context *ctx) {
    // Do nothing
}

// Load instruction
static void proc_ld(cpu_context *ctx) {
    // TODO: Implement
}

// Check condition of set flags
static bool check_cond(cpu_context *ctx) {
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;

    switch (ctx->curr_instr->cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
    }
    
    return false;
}

// Jump instruction
static void proc_jp(cpu_context *ctx) {
    // Check condition used for conditional jumps
    if (check_cond(ctx)) {
        ctx->regs.pc = ctx->fetched_data;
        emu_cycles(1);
    }
}

// Disable interrupts
static void proc_di(cpu_context *ctx) {
    ctx->interrupt_master_enabled = false;
}

// Set flags in F
void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c) {
    // Set zero flag
    if (z != -1) {
        BIT_SET(ctx->regs.f, 7, z);
    }

    // Set subtraction flag
    if (n != -1) {
        BIT_SET(ctx->regs.f, 6, n);
    }

    // Set half carry flag
    if (h != -1) {
        BIT_SET(ctx->regs.f, 5, h);
    }

    // Set carry flag
    if (c != -1) {
        BIT_SET(ctx->regs.f, 4, c);
    }
}

// XOR instruction
static void proc_xor(cpu_context *ctx) {
    ctx->regs.a ^= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

// Processor lookup table
IN_PROC processors[] = {
    [IN_NONE] = proc_none,
    [IN_NOP] = proc_nop,
    [IN_LD] = proc_ld,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di,
    [IN_XOR] = proc_xor,
};

// Get processor for instruction by type
IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}