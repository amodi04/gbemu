#pragma once

#include <common.h>
#include <instructions.h>

// Defines the Gameboy CPU (LR35902) registers
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

    // Current fetched data in the FDE cycle
    u16 fetched_data;
    u16 mem_dest;
    bool dest_is_mem;
    u8 curr_opcode;
    instruction *curr_instr;

    bool halted;
    bool stepping;

    bool interrupt_master_enabled;
    u8 ie_register;

} cpu_context;

// Initializes the CPU
void cpu_init();

// Steps the CPU by one cycle
// Returns true if the CPU is still running
bool cpu_step();

// A function pointer to a function that processes an instruction
typedef void (*IN_PROC)(cpu_context *ctx);

// Returns the processor for a given instruction type
IN_PROC inst_get_processor(in_type type);

// Z Flag: Zero Flag
#define CPU_FLAG_Z BIT(ctx->regs.f, 7)

// C Flag: Carry Flag
#define CPU_FLAG_C BIT(ctx->regs.f, 4)

// Returns the value of the given register
u16 cpu_read_reg(reg_type rt);

// Sets the value of the given register
void cpu_set_reg(reg_type rt, u16 val);

// Returns the value of the given 8-bit register
u8 cpu_read_reg8(reg_type rt);

// Sets the value of the given 8-bit register
void cpu_set_reg8(reg_type rt, u8 val);

// Gets the interrupt enable register
u8 cpu_get_ie_register();

// Sets the interrupt enable register
void cpu_set_ie_register(u8 n);

// Gets the registers
cpu_registers *cpu_get_regs();