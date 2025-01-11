// src/f22.zig
const std = @import("std");

// F22 represents a fixed-point number with 22 bits total
// We'll use 14 bits for the integer part and 8 bits for the fraction
pub const F22 = struct {
    // Internal representation uses 22 bits
    value: i22,

    // Constants for conversion
    const FRACTION_BITS = 8;
    const SCALE = 1 << FRACTION_BITS;

    // Constructor from float
    pub fn fromFloat(float_val: f32) F22 {
        return .{
            .value = @intFromFloat(float_val * @as(f32, @floatFromInt(SCALE))),
        };
    }

    // Convert back to float
    pub fn toFloat(self: F22) f32 {
        return @as(f32, @floatFromInt(self.value)) / @as(f32, @floatFromInt(SCALE));
    }

    // Basic arithmetic operations
    pub fn add(self: F22, other: F22) F22 {
        return .{ .value = @as(i22, @truncate(self.value + other.value)) };
    }

    pub fn sub(self: F22, other: F22) F22 {
        return .{ .value = @as(i22, @truncate(self.value - other.value)) };
    }

    pub fn mul(self: F22, other: F22) F22 {
        const result = @as(i32, self.value) * @as(i32, other.value);
        return .{ .value = @as(i22, @truncate(result >> FRACTION_BITS)) };
    }

    pub fn div(self: F22, other: F22) F22 {
        const shifted = @as(i32, self.value) << FRACTION_BITS;
        return .{ .value = @as(i22, @truncate(@divTrunc(shifted, @as(i32, other.value)))) };
    }
};

// Create a test block to verify our implementation
test "F22 basic operations" {
    const value1 = F22.fromFloat(22.0);
    const value2 = F22.fromFloat(2.0);

    const sum = value1.add(value2);
    const diff = value1.sub(value2);
    const prod = value1.mul(value2);
    const quot = value1.div(value2);

    try std.testing.expectApproxEqAbs(sum.toFloat(), 24.0, 0.01);
    try std.testing.expectApproxEqAbs(diff.toFloat(), 20.0, 0.01);
    try std.testing.expectApproxEqAbs(prod.toFloat(), 44.0, 0.01);
    try std.testing.expectApproxEqAbs(quot.toFloat(), 11.0, 0.01);
}
