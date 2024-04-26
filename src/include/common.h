#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// BIT(a, n) - Get the nth bit of a
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

// BIT_SET(a, n, on) - Set the nth bit of a to on
#define BIT_SET(a, n, on) { if (on) a |= (1 << n); else a &= ~(1 << n);}

// BETWEEN(a, b, c) - Check if a is between b and c
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

void delay(u32 ms);

// Used to mark functions that are not yet implemented
#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }