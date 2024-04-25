#include <cpu.h>
#include <bus.h>
#include <emu.h>

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.pc = 0x100;
}

static void fetch_instruction() {
    ctx.curr_opcode = bus_read(ctx.regs.pc++);
    ctx.curr_instr = instruction_by_opcode(ctx.curr_opcode);

    if (ctx.curr_instr == NULL) {
        printf("Unknown instruction: 0x%02X\n", ctx.curr_opcode);
        exit(-7);
    }
}

static void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

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
            printf("Unknown addressing mode%d\n", ctx.curr_instr->mode);
            exit(-7);
            return;
    }
}

static void execute() {
    printf("\tNot executing yet\n");
}

bool cpu_step() {
    
    // FDE cycle
    if(!ctx.halted) {
        u16 pc = ctx.regs.pc;
        fetch_instruction();
        fetch_data();
        printf("Executing instruction: 0x%02X    PC: %04X\n", ctx.curr_opcode, pc);
        execute();
    }

    return true;
}
