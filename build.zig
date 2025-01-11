const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Create a module for our game_state code
    const game_state_module = b.createModule(.{
        .root_source_file = .{ .cwd_relative = "src/game_state.zig" },
    });

    const exe = b.addExecutable(.{
        .name = "f22-raptor",
        .root_source_file = .{ .cwd_relative = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // Add module dependencies
    exe.root_module.addImport("game_state", game_state_module);

    // Link with SDL2
    exe.linkSystemLibrary("SDL2");
    exe.linkLibC();

    // This creates a run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the game");
    run_step.dependOn(&run_cmd.step);

    b.installArtifact(exe);
}
