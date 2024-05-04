#include <cpu.h>
#include <bus.h>
#include <emu.h>
#include <interrupts.h>
#include <debug.h>
#include <timer.h>

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.pc = 0x100;
    ctx.regs.sp = 0xFFFE;
    *((short *)&ctx.regs.a) = 0xB001;
    *((short *)&ctx.regs.b) = 0x1300;
    *((short *)&ctx.regs.d) = 0xD800;
    *((short *)&ctx.regs.h) = 0x4D01;
    ctx.ie_register = 0;
    ctx.int_flags = 0;
    ctx.interrupt_master_enabled = false;
    ctx.enabling_ime = false;

    timer_get_context()->div = 0xABCC;
}

// Fetches the next instruction
static void fetch_instruction() {
    // Fetch the next opcode and increement the program counter
    ctx.curr_opcode = bus_read(ctx.regs.pc++);
    // Get the instruction from the opcode
    ctx.curr_instr = instruction_by_opcode(ctx.curr_opcode);
}

void fetch_data();

// Execute the current instruction
static void execute() {
    // Get the processor for the current instruction
    IN_PROC proc = inst_get_processor(ctx.curr_instr->type);

    if (!proc) {
        NO_IMPL
    }
    
    // Execute the instruction
    proc(&ctx);
}

bool cpu_step() {
    // FDE cycle
    if(!ctx.halted) {
        u16 pc = ctx.regs.pc;
        fetch_instruction();
        emu_cycles(1);
        fetch_data();

        // Debug info
        char flags[16];
        sprintf(flags, "%c%c%c%c", 
            ctx.regs.f & (1 << 7) ? 'Z' : '-',
            ctx.regs.f & (1 << 6) ? 'N' : '-',
            ctx.regs.f & (1 << 5) ? 'H' : '-',
            ctx.regs.f & (1 << 4) ? 'C' : '-'
        );

        char instr[16];
        instr_to_str(&ctx, instr);

        printf("%08lX - %04X: %-12s (%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n", 
            emu_get_context()->ticks,
            pc, instr, ctx.curr_opcode,
            bus_read(pc + 1), bus_read(pc + 2), ctx.regs.a, flags, ctx.regs.b, ctx.regs.c,
            ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);


        if (ctx.curr_instr == NULL) {
            printf("Unknown instruction! %02X\n", ctx.curr_opcode);
            exit(-7);
        }

        debug_update();
        debug_print();

        execute();
    } else {
        // Halted
        emu_cycles(1);

        if (ctx.int_flags) {
            ctx.halted = false;
        }
    }

    if (ctx.interrupt_master_enabled) {
        cpu_handle_interrupts(&ctx);
        ctx.enabling_ime = false;
    }

    if (ctx.enabling_ime) {
        ctx.interrupt_master_enabled = true;
    }

    return true;
}

u8 cpu_get_ie_register() {
    return ctx.ie_register;
}

void cpu_set_ie_register(u8 n) {
    ctx.ie_register = n;
}

void cpu_request_interrupt(interrupt_type t) {
    ctx.int_flags |= t;
}