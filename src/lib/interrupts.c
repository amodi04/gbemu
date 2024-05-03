#include <cpu.h>
#include <stack.h>
#include <interrupts.h>

// Handles a specific interrupt
void interrupt_handle(cpu_context *ctx, u16 addr) {
    // Push the current PC onto the stack
    stack_push16(ctx->regs.pc);

    // Jump to the interrupt handler
    ctx->regs.pc = addr;
}

// Checks if an interrupt is enabled and requested
bool int_check(cpu_context *ctx, u16 address, interrupt_type it) {
    if (ctx->int_flags & it && ctx->ie_register & it) {
        interrupt_handle(ctx, address);
        ctx->int_flags &= ~it;
        ctx->halted = false;
        ctx->interrupt_master_enabled = false;

        return true;
    }

    return false;
}

void cpu_handle_interrupts(cpu_context *ctx) {
    if (int_check(ctx, 0x40, INT_VBLANK)) {

    } else if (int_check(ctx, 0x48, INT_LCD_STAT)) {

    } else if (int_check(ctx, 0x50, INT_TIMER)) {

    }  else if (int_check(ctx, 0x58, INT_SERIAL)) {

    }  else if (int_check(ctx, 0x60, INT_JOYPAD)) {

    } 
}