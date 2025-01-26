#ifndef F22_H
#define F22_H

#include <stdint.h>

// F22 represents a fixed-point number with 22 bits total
// 14 bits for integer part, 8 bits for fraction
typedef struct {
    int32_t value;  // using 32-bit to avoid truncation issues in arithmetic
} F22;

// Constants for conversion
#define F22_FRACTION_BITS 8
#define F22_SCALE (1 << F22_FRACTION_BITS)

// Constructor and conversion functions
F22 f22_from_float(float float_val);
float f22_to_float(F22 val);

// Basic arithmetic operations
F22 f22_add(F22 a, F22 b);
F22 f22_sub(F22 a, F22 b);
F22 f22_mul(F22 a, F22 b);
F22 f22_div(F22 a, F22 b);

#endif // F22_H
