const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard release options
    const optimize = b.standardOptimizeOption(.{});
    const windows_build = if (b.option(bool, "windows", "Build for windows")) |w| w else true;

    // Add a new option for web builds
    const web_build = if (b.option(bool, "web", "Build for web (WASM)")) |w| w else false;
    const options = b.addOptions();
    options.addOption(bool, "web_build", web_build);

    if (!web_build) {
        // Native build (non-WASM)
        const target = b.standardTargetOptions(.{});
        const exe = b.addExecutable(.{
            .name = "typeroo",
            .root_source_file = .{ .cwd_relative = "src/main.zig" },
            .target = target,
            .optimize = optimize,
        });

        // Add build options
        exe.root_module.addOptions("build_options", options);

        // Add include paths
        exe.root_module.addIncludePath(.{ .cwd_relative = "src" });

        if (!web_build) {
            exe.root_module.addIncludePath(.{ .cwd_relative = "dependencies/gl" });
            exe.root_module.addIncludePath(.{ .cwd_relative = "dependencies/stb_truetype-1.24" });
            exe.addCSourceFile(.{
                .file = .{ .cwd_relative = "dependencies/gl/glad.c" },
                .flags = &[_][]const u8{"-std=c99"},
            });
            exe.addCSourceFile(.{
                .file = .{ .cwd_relative = "dependencies/stb_truetype-1.24/stb_truetype_impl.c" },
                .flags = &[_][]const u8{"-std=c99"},
            });
        }

        if (windows_build) {
            exe.root_module.addIncludePath(.{ .cwd_relative = "C:/SDL2/include" });
            exe.addLibraryPath(.{ .cwd_relative = "C:/SDL2/lib/x64" });
            exe.addLibraryPath(.{ .cwd_relative = "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64" });
            b.installBinFile("C:/SDL2/lib/x64/SDL2.dll", "SDL2.dll");
        }

        // Link libraries
        exe.linkSystemLibrary("sdl2");
        if (windows_build) {
            exe.linkSystemLibrary("OpenGL32");
        } else {
            exe.linkSystemLibrary("OpenGL");
        }
        exe.linkLibC();

        // Install the executable
        b.installArtifact(exe);

        // Run step
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }

        const run_step = b.step("run", "Run the app");
        run_step.dependOn(&run_cmd.step);
    } else {
        // Web build (WASM)
        const target = std.Target.Query{
            .cpu_arch = .wasm32,
            .os_tag = .emscripten,
        };
        const exe = b.addSharedLibrary(.{
            .name = "typeroo",
            .root_source_file = .{ .cwd_relative = "src/main.zig" },
            .target = b.resolveTargetQuery(target),
            .optimize = optimize,
        });

        // Add build options
        exe.root_module.addOptions("build_options", options);

        // Add include paths
        exe.root_module.addIncludePath(.{ .cwd_relative = "src" });

        // Link SDL2 for WebAssembly
        exe.linkSystemLibrary("SDL2");

        // Add Emscripten-specific include and library paths
        const emscripten_path = std.process.getEnvVarOwned(b.allocator, "EMSDK") catch |err| {
            std.debug.print("Failed to get EMSDK environment variable: {}\n", .{err});
            return;
        };
        defer b.allocator.free(emscripten_path);

        const sysroot = b.pathJoin(&.{ emscripten_path, "upstream/emscripten/cache/sysroot" });
        exe.root_module.addIncludePath(.{ .cwd_relative = b.pathJoin(&.{ sysroot, "include" }) });
        exe.addLibraryPath(.{ .cwd_relative = b.pathJoin(&.{ sysroot, "lib/wasm32-emscripten" }) });

        // Add Emscripten runtime object file
        exe.addObjectFile(.{ .cwd_relative = b.pathJoin(&.{ sysroot, "lib/wasm32-emscripten/crt1.o" }) });

        // Explicitly link libc for Emscripten
        exe.linkLibC();

        // Install the WASM output
        b.installArtifact(exe);

        // Copy the generated files to a web directory (optional)
        const copy_wasm = b.addInstallFile(exe.getEmittedBin(), "web/typeroo.wasm");
        const copy_js = b.addInstallFile(exe.getEmittedBin(), "web/typeroo.js");

        // Build step
        const build_step = b.step("build", "Build the application");
        build_step.dependOn(&exe.step);
        build_step.dependOn(&copy_wasm.step);
        build_step.dependOn(&copy_js.step);

        // Run step (optional, for testing)
        const run_step = b.step("run", "Run the application");
        const run_cmd = b.addSystemCommand(&.{ "emrun", "web/typeroo.html" });
        run_cmd.step.dependOn(build_step);
        run_step.dependOn(&run_cmd.step);
    }
}
