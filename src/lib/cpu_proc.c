#include <cpu.h>
#include <emu.h>
#include <bus.h>
#include <stack.h>

// Processes CPU instructions

// Set flags in F register
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
    if (ctx->dest_is_mem) {
        // LD (BC), A

        // If 16-bit register
        if (ctx->curr_instr->reg_2 >= RT_AF) {
            emu_cycles(1);
            bus_write16(ctx->mem_dest, ctx->fetched_data);
        } else {
            bus_write(ctx->mem_dest, ctx->fetched_data);
        }
        emu_cycles(1);
        return;
    }

    // Load stack pointer + 8-bit immediate into memory
    if (ctx->curr_instr->mode == AM_HL_SPR) {
        // Check if H flag should be set
        u8 hflag = (cpu_read_reg(ctx->curr_instr->reg_2) & 0xF) + 
            (ctx->fetched_data & 0xF) >= 0x10;

        // Check if C flag should be set
        u8 cflag = (cpu_read_reg(ctx->curr_instr->reg_2) & 0xFF) + 
            (ctx->fetched_data & 0xFF) >= 0x100;

        cpu_set_flags(ctx, 0, 0, hflag, cflag);
        cpu_set_reg(ctx->curr_instr->reg_1, 
            cpu_read_reg(ctx->curr_instr->reg_2) + (char)ctx->fetched_data);

        return;
    }

    cpu_set_reg(ctx->curr_instr->reg_1, ctx->fetched_data);
}

// Load into hram
static void proc_ldh(cpu_context *ctx) {
    if (ctx->curr_instr->reg_1 == RT_A) {
        // Set A to value at address hram
        cpu_set_reg(ctx->curr_instr->reg_1, bus_read(0xFF00 | ctx->fetched_data));
    } else {
        // Set value at address hram to A
        bus_write(0xFF00 | ctx->fetched_data, ctx->regs.a);
    }

    emu_cycles(1);
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

// Goto address
static void goto_addr(cpu_context *ctx, u16 addr, bool pushpc) {
    // Check condition used for conditional jumps
    if (check_cond(ctx)) {
        if (pushpc) {
            emu_cycles(2);
            stack_push16(ctx->regs.pc);
        }
        ctx->regs.pc = addr;
        emu_cycles(1);
    }
}

// Jump instruction
static void proc_jp(cpu_context *ctx) {
    goto_addr(ctx, ctx->fetched_data, false);
}

// Jump relative instruction (add immediate to PC)
static void proc_jr(cpu_context *ctx) {
    // Relative address could be pos or neg
    char rel = (char)(ctx->fetched_data & 0xFF);
    u16 addr = ctx->regs.pc + rel;
    goto_addr(ctx, addr, false);
}


// Call instruction (push PC to stack, jump to immediate address)
static void proc_call(cpu_context *ctx) {
    goto_addr(ctx, ctx->fetched_data, true);
}

// RST instruction (push PC to stack, jump to immediate address)
static void proc_rst(cpu_context *ctx) {
    goto_addr(ctx, ctx->curr_instr->param, true);
}

// Ret instruction (opposite of call)
static void proc_ret(cpu_context *ctx) {
    if (ctx->curr_instr->cond != CT_NONE) {
        emu_cycles(1);
    }

    if (check_cond(ctx)) {
        u16 lo = stack_pop();
        emu_cycles(1);
        u16 hi = stack_pop();
        emu_cycles(1);

        u16 n = (hi << 8) | lo;
        ctx->regs.pc = n;

        emu_cycles(1);
    }
}

// Return from interrupt
static void proc_reti(cpu_context *ctx) {
    ctx->interrupt_master_enabled = true;
    proc_ret(ctx);
}

// Pop from stack
static void proc_pop(cpu_context *ctx) {
    u16 lo = stack_pop();
    emu_cycles(1);
    u16 hi = stack_pop();
    emu_cycles(1);

    u16 n = (hi << 8) | lo;

    cpu_set_reg(ctx->curr_instr->reg_1, n);

    if (ctx->curr_instr->reg_1 == RT_AF) {
        // Mask out lower 4 bits because they are reserved for flags
        cpu_set_reg(ctx->curr_instr->reg_1, n & 0xFFF0);
    }
}

// Push to stack
static void proc_push(cpu_context *ctx) {
    u16 hi = (cpu_read_reg(ctx->curr_instr->reg_1) >> 8) & 0xFF;
    emu_cycles(1);
    stack_push(hi);

    u16 lo = cpu_read_reg(ctx->curr_instr->reg_2) & 0xFF;
    emu_cycles(1);
    stack_push(lo);
    
    emu_cycles(1);
}

// Disable interrupts
static void proc_di(cpu_context *ctx) {
    ctx->interrupt_master_enabled = false;
}

// XOR instruction
static void proc_xor(cpu_context *ctx) {
    ctx->regs.a ^= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

// Processor lookup table
static IN_PROC processors[] = {
    [IN_NONE] = proc_none,
    [IN_NOP] = proc_nop,
    [IN_LD] = proc_ld,
    [IN_LDH] = proc_ldh,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di,
    [IN_POP] = proc_pop,
    [IN_PUSH] = proc_push,
    [IN_JR] = proc_jr,
    [IN_CALL] = proc_call,
    [IN_RET] = proc_ret,
    [IN_RST] = proc_rst,
    [IN_RETI] = proc_reti,
    [IN_XOR] = proc_xor
};

// Get processor for instruction by type
IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}