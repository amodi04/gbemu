#include <cpu.h>
#include <bus.h>
#include <emu.h>

extern cpu_context ctx;

// Fetches the data for the current instruction
void fetch_data() {
    // Reset the memory destination
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

        case AM_R_R:
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_2);

        case AM_R_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_R_D16: // Same as AM_16
        case AM_D16: {
            // Reading 16-bit data so two cycles needed
            // (8 bits at a time)
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);
            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            // Combine the data with logical OR
            ctx.fetched_data = lo | (hi << 8);
            ctx.regs.pc += 2;

            return;
        }

        case AM_MR_R:
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.curr_instr->reg_1);
            ctx.dest_is_mem = true;

            // C register is 8-bit so OR with 0xFF00 to make 16-bit
            if (ctx.curr_instr->reg_1 == RT_C) {
                ctx.mem_dest |= 0xFF00;
            }

            return;

        case AM_R_MR:
            u16 addr = cpu_read_reg(ctx.curr_instr->reg_2);

            // C register is 8-bit so OR with 0xFF00 to make 16-bit
            if (ctx.curr_instr->reg_1 == RT_C) {
                addr |= 0xFF00;
            }

            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);
            return;

        case AM_R_HLI:
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.curr_instr->reg_2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            return;

        case AM_R_HLD:
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.curr_instr->reg_2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            return;

        case AM_HLI_R:
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.curr_instr->reg_1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            return;

        case AM_HLD_R:
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.curr_instr->reg_1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            return;

        case AM_R_A8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_A8_R:
            // Register to 8-bit so OR with 0xFF00 to make 16-bit
            ctx.mem_dest = bus_read(ctx.regs.pc) | 0xFF00;
            ctx.dest_is_mem = true;
            ctx.regs.pc++;
            return;

        case AM_HL_SPR:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_A16_R: // Same as AM_D16_R
        case AM_D16_R: {
            // Reading 16-bit data so two cycles needed
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.mem_dest = lo | (hi << 8);
            ctx.dest_is_mem = true;

            ctx.regs.pc += 2;
            ctx.fetched_data = cpu_read_reg(ctx.curr_instr->reg_2);
        } return;

        case AM_MR_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            ctx.mem_dest = cpu_read_reg(ctx.curr_instr->reg_1);
            ctx.dest_is_mem = true;
            return;

        case AM_MR:
            ctx.mem_dest = cpu_read_reg(ctx.curr_instr->reg_1);
            ctx.dest_is_mem = true;
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.curr_instr->reg_1));
            emu_cycles(1);
            return;

        case AM_R_A16: {
            // Reading 16-bit data so two cycles needed
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            u16 addr = lo | (hi << 8);

            ctx.regs.pc += 2;
            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);

            return;
        }

        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", ctx.curr_instr->mode, ctx.curr_opcode);
            exit(-7);
            return;
    }
}