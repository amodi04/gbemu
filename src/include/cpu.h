#pragma once

#include <common.h>
#include <instructions.h>

typedef struct {
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    u16 sp;
    u16 pc;
} cpu_registers;

typedef struct {
    cpu_registers regs;

    // Current fetch...
    u16 fetched_data;
    u16 mem_dest;
    bool dest_is_mem;
    u8 curr_opcode;
    instruction *curr_instr;

    bool halted;
    bool stepping;

    bool interrupt_master_enabled;

} cpu_context;

void cpu_init();
bool cpu_step();

typedef void (*IN_PROC)(cpu_context *ctx);

IN_PROC inst_get_processor(in_type type);

#define CPU_FLAG_Z BIT(ctx->regs.f, 7)
#define CPU_FLAG_C BIT(ctx->regs.f, 4)

u16 cpu_read_reg(reg_type rt);