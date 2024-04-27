#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init() {
    // Set the program counter to 0x100
    // GB ROMs start at 0x100
    ctx.regs.pc = 0x100;
    ctx.regs.a = 0x01;
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
        fetch_data();

        // Debug info
        printf("%04X: %-7s (%02X %02X %02X) A: %02X BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
            pc,
            inst_name(ctx.curr_instr->type),
            ctx.curr_opcode,
            bus_read(pc + 1), // Next address
            bus_read(pc + 2), // Next address
            ctx.regs.a,
            ctx.regs.b, ctx.regs.c,
            ctx.regs.d, ctx.regs.e,
            ctx.regs.h, ctx.regs.l
        );

        if (ctx.curr_instr == NULL) {
            printf("Unknown intruction! %02X\n", ctx.curr_opcode);
            exit(-7);
        }

        execute();
    }

    return true;
}

u8 cpu_get_ie_register() {
    return ctx.ie_register;
}

void cpu_set_ie_register(u8 n) {
    ctx.ie_register = n;
}
