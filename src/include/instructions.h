#pragma once

#include <common.h>

// Defines the Gameboy CPU (LR35902) instruction set

// Addressing modes
typedef enum {
    AM_IMP,    // Implied
    AM_R_D16,  // 16-bit immediate to Register
    AM_R_R,    // Register to Register
    AM_MR_R,   // Register to Memory
    AM_R,      // Register
    AM_R_D8,   // 8-bit immediate to Register
    AM_R_MR,   // Memory to Register
    AM_R_HLI,  // Memory at HL, increment, to Register
    AM_R_HLD,  // Memory at HL, decrement, to Register
    AM_HLI_R,  // Register to Memory at HL, increment
    AM_HLD_R,  // Register to Memory at HL, decrement
    AM_R_A8,   // Memory at 0xFF00 + 8-bit immediate to Register
    AM_A8_R,   // Register to Memory at 0xFF00 + 8-bit immediate
    AM_HL_SPR, // Stack Pointer location to HL
    AM_D16,    // 16-bit immediate
    AM_D8,     // 8-bit immediate
    AM_D16_R,  // Register to 16-bit immediate 
    AM_MR_D8,  // 8-bit immediate to Memory
    AM_MR,     // Memory
    AM_A16_R,  // Register to Memory at 16-bit immediate
    AM_R_A16   // Memory at 16-bit immediate to Register
} addr_mode;

// Register types
typedef enum {
    RT_NONE, // No register
    RT_A,    // A register (Accumulator)
    RT_F,    // F register (Flags)
    RT_B,    // B register
    RT_C,    // C register
    RT_D,    // D register
    RT_E,    // E register
    RT_H,    // H register
    RT_L,    // L register
    RT_AF,   // AF register pair
    RT_BC,   // BC register pair
    RT_DE,   // DE register pair
    RT_HL,   // HL register pair
    RT_SP,   // Stack Pointer register
    RT_PC    // Program Counter register
} reg_type;

// Instruction types
typedef enum {
    IN_NONE,  // No instruction
    IN_NOP,   // No operation
    IN_LD,    // Load
    IN_INC,   // Increment
    IN_DEC,   // Decrement
    IN_RLCA,  // Rotate Left through Carry register A
    IN_ADD,   // Add
    IN_RRCA,  // Rotate Right through Carry register A
    IN_STOP,  // Stop
    IN_RLA,   // Rotate Left register A
    IN_JR,    // Jump Relative
    IN_RRA,   // Rotate Right register A
    IN_DAA,   // Decimal Adjust Accumulator
    IN_CPL,   // Complement Accumulator
    IN_SCF,   // Set Carry Flag
    IN_CCF,   // Complement Carry Flag
    IN_HALT,  // Halt
    IN_ADC,   // Add with Carry
    IN_SUB,   // Subtract
    IN_SBC,   // Subtract with Carry
    IN_AND,   // Logical AND
    IN_XOR,   // Logical XOR
    IN_OR,    // Logical OR
    IN_CP,    // Compare
    IN_POP,   // Pop from stack into register pair
    IN_JP,    // Jump
    IN_PUSH,  // Push
    IN_RET,   // Pop from stack into PC
    IN_CB,    // Bitwise operations
    IN_CALL,  // Call (push PC to stack, jump to immediate address)
    IN_RETI,  // Return from interrupt
    IN_LDH,   // Load to/from high memory
    IN_JPHL,  // Jump to HL
    IN_DI,    // Disable interrupts
    IN_EI,    // Enable interrupts
    IN_RST,   // Restart
    IN_ERR,   // Error
    //CB instructions...
    IN_RLC,   // Rotate Left through Carry
    IN_RRC,   // Rotate Right through Carry
    IN_RL,    // Rotate Left
    IN_RR,    // Rotate Right
    IN_SLA,   // Shift Left Arithmetic
    IN_SRA,   // Shift Right Arithmetic
    IN_SWAP,  // Swap nibbles
    IN_SRL,   // Shift Right Logical
    IN_BIT,   // Test bit
    IN_RES,   // Reset bit
    IN_SET    // Set bit
} in_type;

// Condition types
typedef enum {
    CT_NONE, // No condition
    CT_NZ,   // Not Zero
    CT_Z,    // Zero
    CT_NC,   // Not Carry
    CT_C     // Carry
} cond_type;

// Instruction information
typedef struct {
    in_type type;   // Instruction type
    addr_mode mode; // Addressing mode
    reg_type reg_1; // First register
    reg_type reg_2; // Second register
    cond_type cond; // Condition
    u8 param;       // Additional parameter
} instruction;

// Returns the instruction given by the opcode
instruction *instruction_by_opcode(u8 opcode);

// Returns the name of the instruction given by the type
char *inst_name(in_type t);