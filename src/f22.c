#include "f22.h"

F22 f22_from_float(float float_val) {
    F22 result;
    result.value = (int32_t)(float_val * F22_SCALE);
    return result;
}

float f22_to_float(F22 val) {
    return (float)val.value / F22_SCALE;
}

F22 f22_add(F22 a, F22 b) {
    F22 result;
    result.value = a.value + b.value;
    return result;
}

F22 f22_sub(F22 a, F22 b) {
    F22 result;
    result.value = a.value - b.value;
    return result;
}

F22 f22_mul(F22 a, F22 b) {
    F22 result;
    // Use 64-bit intermediate to avoid overflow
    result.value = (int32_t)(((int64_t)a.value * b.value) >> F22_FRACTION_BITS);
    return result;
}

F22 f22_div(F22 a, F22 b) {
    F22 result;
    // Shift left first to maintain precision
    result.value = (int32_t)((((int64_t)a.value << F22_FRACTION_BITS) / b.value));
    return result;
}