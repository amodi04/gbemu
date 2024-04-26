#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.pc = 0x100;
    ctx.regs.a = 0x01;
}

static void fetch_instruction() {
    ctx.curr_opcode = bus_read(ctx.regs.pc++);
    ctx.curr_instr = instruction_by_opcode(ctx.curr_opcode);
}

static void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

    if (ctx.curr_instr == NULL) {
        return;
    }

    switch(ctx.curr_instr->mode) {
        case AM_IMP:
            return;
        case AM_R:
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_1);
            return;
        case AM_R_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        case AM_D16: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);
            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.fetched_data = lo | (hi << 8);
            ctx.regs.pc += 2;

            return;
        }

        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", ctx.curr_instr->mode, ctx.curr_opcode);
            exit(-7);
            return;
    }
}

static void execute() {
    IN_PROC proc = inst_get_processor(ctx.curr_instr->type);

    if (!proc) {
        NO_IMPL
    }

    proc(&ctx);
}

bool cpu_step() {
    
    // FDE cycle
    if(!ctx.halted) {
        u16 pc = ctx.regs.pc;
        fetch_instruction();
        fetch_data();

        printf("%04X: %-7s (%02X %02X %02X) A: %02X B: %02X C: %02X\n",
            pc,
            inst_name(ctx.curr_instr->type),
            ctx.curr_opcode,
            bus_read(pc + 1),
            bus_read(pc + 2),
            ctx.regs.a,
            ctx.regs.b,
            ctx.regs.c
        );

        if (ctx.curr_instr == NULL) {
            printf("Unknown intruction! %02X\n", ctx.curr_opcode);
            exit(-7);
        }

        execute();
    }

    return true;
}
