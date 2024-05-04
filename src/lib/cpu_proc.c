#include <bus.h>
#include <cpu.h>
#include <emu.h>
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

// Ordered by how the logical operations are stored in the instruction set
reg_type rt_lookup[] = {
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A};

// Decode register from bits
reg_type decode_reg(u8 reg) {
    // Only 3 bits are used for register
    if (reg > 0b111) {
        return RT_NONE;
    }

    return rt_lookup[reg];
}

// Process CB (bitwise) instructions
// Instruction are now identified by a prefix byte
static void proc_cb(cpu_context *ctx) {
    u8 op = ctx->fetched_data;
    reg_type reg = decode_reg(op & 0b111);
    // Get bit to operate on
    u8 bit = (op >> 3) & 0b111;

    // Get bit operation
    u8 bit_op = (op >> 6) & 0b11;

    // Get register value
    u8 reg_val = cpu_read_reg8(reg);

    emu_cycles(1);

    if (reg == RT_HL) {
        emu_cycles(2);
    }

    switch (bit_op) {
    // BIT
    case 1:
        // Set flags depending on bit value
        cpu_set_flags(ctx, !(reg_val & (1 << bit)), 0, 1, -1);
        return;
    // RES
    case 2:
        // This resets the bit to 0
        reg_val &= ~(1 << bit);
        cpu_set_reg8(reg, reg_val);
        return;
    // SET
    case 3:
        // This sets the bit to 1
        reg_val |= (1 << bit);
        cpu_set_reg8(reg, reg_val);
        return;
    }

    // RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL operations
    bool flagC = CPU_FLAG_C;

    switch (bit) {
    case 0: {
        // RLC
        bool setC = false;
        u8 result = (reg_val << 1) & 0xFF;

        if ((reg_val & (1 << 7)) != 0) {
            result |= 1;
            setC = true;
        }

        cpu_set_reg8(reg, result);
        cpu_set_flags(ctx, result == 0, false, false, setC);
    }
        return;

    case 1: {
        // RRC - Rotate right with carry
        u8 old = reg_val;
        reg_val >>= 1;
        reg_val |= (old << 7);

        cpu_set_reg8(reg, reg_val);
        cpu_set_flags(ctx, !reg_val, false, false, old & 1);
    }
        return;

    // RL - Rotate left
    case 2: {
        u8 old = reg_val;
        reg_val <<= 1;
        reg_val |= flagC;

        cpu_set_reg8(reg, reg_val);
        cpu_set_flags(ctx, !reg_val, false, false, !!(old & 0x80));
    }
        return;

    // RR - Rotate right
    case 3: {
        u8 old = reg_val;
        reg_val >>= 1;

        reg_val |= (flagC << 7);

        cpu_set_reg8(reg, reg_val);
        cpu_set_flags(ctx, !reg_val, false, false, old & 1);
    }
        return;

    // SLA - Shift left arithmetic
    case 4: {
        u8 old = reg_val;
        reg_val <<= 1;

        cpu_set_reg8(reg, reg_val);
        cpu_set_flags(ctx, !reg_val, false, false, !!(old & 0x80));
    }
        return;

    // SRA - Shift right arithmetic
    case 5: {
        u8 u = (int8_t)reg_val >> 1;
        cpu_set_reg8(reg, u);
        cpu_set_flags(ctx, !u, 0, 0, reg_val & 1);
    }
        return;

    // SWAP - Swap nibbles
    case 6: {
        reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
        cpu_set_reg8(reg, reg_val);
        cpu_set_flags(ctx, reg_val == 0, false, false, false);
    }
        return;

    // SRL - Shift right logical
    case 7: {
        u8 u = reg_val >> 1;
        cpu_set_reg8(reg, u);
        cpu_set_flags(ctx, !u, 0, 0, reg_val & 1);
    }
        return;
    }

    fprintf(stderr, "Unknown CB instruction: %02X\n", op);
    NO_IMPL
}

// Rotate left through carry
static void proc_rlca(cpu_context *ctx) {
    u8 u = ctx->regs.a;

    // Calculate if carry should be set
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    ctx->regs.a = u;

    cpu_set_flags(ctx, u == 0, 0, 0, c);
}

// Rotate right through carry
static void proc_rrca(cpu_context *ctx) {
    u8 b = ctx->regs.a;
    ctx->regs.a >>= 1;

    // Shift last to front
    ctx->regs.a |= (b << 7);

    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, b & 1);
}

// Rotate left
static void proc_rla(cpu_context *ctx) {
    u8 u = ctx->regs.a;
    u8 cf = CPU_FLAG_C;
    u8 c = (u >> 7) & 1;

    ctx->regs.a = (u << 1) | cf;
    cpu_set_flags(ctx, 0, 0, 0, c);
    
}

// Rotate right
static void proc_rra(cpu_context *ctx) {
    u8 cf = CPU_FLAG_C;

    // Is first bit set?
    u8 new_cf = ctx->regs.a & 1;

    ctx->regs.a >>= 1;
    ctx->regs.a |= (cf << 7);

    cpu_set_flags(ctx, 0, 0, 0, new_cf);
}

// Stop instruction
static void proc_stop(cpu_context *ctx) {
    fprintf(stderr, "STOPPING\n");
    NO_IMPL
}

// Decimal adjust accumulator - Don't fully understand this
static void proc_daa(cpu_context *ctx) {
    u8 u = 0;
    int cf = 0;

    if (CPU_FLAG_H || (!CPU_FLAG_N && (ctx->regs.a & 0xF) > 9)) {
        u = 6;
    }

    if (CPU_FLAG_C || (!CPU_FLAG_N && ctx->regs.a > 0x99)) {
        u |= 0x60;
        cf = 1;
    }

    ctx->regs.a += CPU_FLAG_N ? -u : u;

    cpu_set_flags(ctx, ctx->regs.a == 0, -1, 0, cf);
}

// Complement accumulator
static void proc_cpl(cpu_context *ctx) {
    // Negate accumulator and set flags
    ctx->regs.a = ~ctx->regs.a;
    cpu_set_flags(ctx, 0, 1, 1, -1);
}

// Set carry flag
static void proc_scf(cpu_context *ctx) {
    cpu_set_flags(ctx, -1, 0, 0, 1);
}

// Complement carry flag
static void proc_ccf(cpu_context *ctx) {
    // Negate carry flag
    cpu_set_flags(ctx, -1, 0, 0, CPU_FLAG_C ^ 1);
}

// Halt instruction
static void proc_halt(cpu_context *ctx) {
    ctx->halted = true;
}

// AND instruction
static void proc_and(cpu_context *ctx) {
    ctx->regs.a &= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 1, 0);
}

// XOR instruction
static void proc_xor(cpu_context *ctx) {
    ctx->regs.a ^= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

// OR instruction
static void proc_or(cpu_context *ctx) {
    ctx->regs.a |= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

static void proc_cp(cpu_context *ctx) {
    int n = (int)ctx->regs.a - (int)ctx->fetched_data;

    cpu_set_flags(ctx, n == 0, 1,
                  ((int)ctx->regs.a & 0x0F) - ((int)ctx->fetched_data & 0xF) < 0, n < 0);
}

// Check if register is 16-bit
static bool is_16_bit(reg_type rt) {
    return rt >= RT_AF;
}

// Load instruction
static void proc_ld(cpu_context *ctx) {
    if (ctx->dest_is_mem) {
        // LD (BC), A

        // If 16-bit register
        if (is_16_bit(ctx->curr_instr->reg_2)) {
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
                       (ctx->fetched_data & 0xF) >=
                   0x10;

        // Check if C flag should be set
        u8 cflag = (cpu_read_reg(ctx->curr_instr->reg_2) & 0xFF) +
                       (ctx->fetched_data & 0xFF) >=
                   0x100;

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
    case CT_NONE:
        return true;
    case CT_C:
        return c;
    case CT_NC:
        return !c;
    case CT_Z:
        return z;
    case CT_NZ:
        return !z;
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

// Enable interrupts
static void proc_ei(cpu_context *ctx) {
    ctx->enabling_ime = true;
}

// Increment register
static void proc_inc(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->curr_instr->reg_1) + 1;

    if (is_16_bit(ctx->curr_instr->reg_1)) {
        emu_cycles(1);
    }

    // If HL and mode is memory read then we need to first read from memory
    if (ctx->curr_instr->reg_1 == RT_HL && ctx->curr_instr->mode == AM_MR) {
        val = bus_read(cpu_read_reg(RT_HL)) + 1;
        val &= 0xFF;
        bus_write(cpu_read_reg(RT_HL), val);
    } else {
        cpu_set_reg(ctx->curr_instr->reg_1, val);
        val = cpu_read_reg(ctx->curr_instr->reg_1);
    }

    // 0x03 operations don't set flags
    if ((ctx->curr_opcode & 0x03) == 0x03) {
        return;
    }

    cpu_set_flags(ctx, val == 0, 0, (val & 0x0F) == 0, -1);
}

// Decrement register
static void proc_dec(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->curr_instr->reg_1) - 1;

    if (is_16_bit(ctx->curr_instr->reg_1)) {
        emu_cycles(1);
    }

    // If HL and mode is memory read then we need to first read from memory
    if (ctx->curr_instr->reg_1 == RT_HL && ctx->curr_instr->mode == AM_MR) {
        val = bus_read(cpu_read_reg(RT_HL)) - 1;
        bus_write(cpu_read_reg(RT_HL), val);
    } else {
        cpu_set_reg(ctx->curr_instr->reg_1, val);
        val = cpu_read_reg(ctx->curr_instr->reg_1);
    }

    // 0x0B operations don't set flags
    if ((ctx->curr_opcode & 0x0B) == 0x0B) {
        return;
    }

    cpu_set_flags(ctx, val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

// Subtract instruction
static void proc_sub(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->curr_instr->reg_1) - ctx->fetched_data;

    int z = val == 0;
    int h = ((int)cpu_read_reg(ctx->curr_instr->reg_1) & 0xF) - ((int)ctx->fetched_data & 0xF) < 0;
    int c = ((int)cpu_read_reg(ctx->curr_instr->reg_1)) - ((int)ctx->fetched_data) < 0;

    cpu_set_reg(ctx->curr_instr->reg_1, val);
    cpu_set_flags(ctx, z, 1, h, c);
}

// Subtract with carry
static void proc_sbc(cpu_context *ctx) {
    u8 val = ctx->fetched_data + CPU_FLAG_C;

    int z = cpu_read_reg(ctx->curr_instr->reg_1) - val == 0;

    int h = ((int)cpu_read_reg(ctx->curr_instr->reg_1) & 0xF) - ((int)ctx->fetched_data & 0xF) - ((int)CPU_FLAG_C) < 0;
    int c = ((int)cpu_read_reg(ctx->curr_instr->reg_1)) - ((int)ctx->fetched_data) - ((int)CPU_FLAG_C) < 0;

    cpu_set_reg(ctx->curr_instr->reg_1, cpu_read_reg(ctx->curr_instr->reg_1) - val);
    cpu_set_flags(ctx, z, 1, h, c);
}

// Add with carry
static void proc_adc(cpu_context *ctx) {
    u16 u = ctx->fetched_data;
    u16 a = ctx->regs.a;
    u16 c = CPU_FLAG_C;

    ctx->regs.a = a + u + c;

    cpu_set_flags(ctx, ctx->regs.a == 0, 0, (a & 0xF) + (u & 0xF) + c > 0xF, a + u + c > 0xFF);
}

// Add instruction
static void proc_add(cpu_context *ctx) {
    // 32-bit because there could be overflow when adding 16-bit values
    u32 val = cpu_read_reg(ctx->curr_instr->reg_1) + ctx->fetched_data;

    bool is_16bit = is_16_bit(ctx->curr_instr->reg_1);

    if (is_16bit) {
        emu_cycles(1);
    }

    // Fetched value could be negative if we are adding to SP
    if (ctx->curr_instr->reg_1 == RT_SP) {
        val = cpu_read_reg(ctx->curr_instr->reg_1) + (char)ctx->fetched_data;
    }

    int z = (val & 0xFF) == 0;
    // Half carry if result is greater than a nibble
    int h = (cpu_read_reg(ctx->curr_instr->reg_1) & 0xF) + (ctx->fetched_data & 0xF) >= 0x10;

    // Carry if result is greater than a byte
    int c = (int)(cpu_read_reg(ctx->curr_instr->reg_1) & 0xFF) + (int)(ctx->fetched_data & 0xFF) >= 0x100;

    if (is_16bit) {
        // Z unchanged
        z = -1;

        // Half carry if result is greater than 3 nibbles
        h = (cpu_read_reg(ctx->curr_instr->reg_1) & 0xFFF) + (ctx->fetched_data & 0xFFF) >= 0x1000;
        u32 n = ((u32)cpu_read_reg(ctx->curr_instr->reg_1)) + ((u32)ctx->fetched_data);

        // Carry if result is greater than 16 bits
        c = n >= 0x10000;
    }

    if (ctx->curr_instr->reg_1 == RT_SP) {
        z = 0;
        h = (cpu_read_reg(ctx->curr_instr->reg_1) & 0xF) + (ctx->fetched_data & 0xF) >= 0x10;

        // Carry if 0x100 because we are adding a signed 8-bit value
        c = (int)(cpu_read_reg(ctx->curr_instr->reg_1) & 0xFF) + (int)(ctx->fetched_data & 0xFF) >= 0x100;
    }

    cpu_set_reg(ctx->curr_instr->reg_1, val & 0xFFFF);
    cpu_set_flags(ctx, z, 0, h, c);
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
    [IN_INC] = proc_inc,
    [IN_DEC] = proc_dec,
    [IN_ADD] = proc_add,
    [IN_ADC] = proc_adc,
    [IN_SUB] = proc_sub,
    [IN_SBC] = proc_sbc,
    [IN_AND] = proc_and,
    [IN_XOR] = proc_xor,
    [IN_OR] = proc_or,
    [IN_CP] = proc_cp,
    [IN_CB] = proc_cb,
    [IN_RLCA] = proc_rlca,
    [IN_RRCA] = proc_rrca,
    [IN_RLA] = proc_rla,
    [IN_RRA] = proc_rra,
    [IN_STOP] = proc_stop,
    [IN_HALT] = proc_halt,
    [IN_DAA] = proc_daa,
    [IN_CPL] = proc_cpl,
    [IN_SCF] = proc_scf,
    [IN_CCF] = proc_ccf,
    [IN_EI] = proc_ei,
    [IN_RETI] = proc_reti,
};

// Get processor for instruction by type
IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}