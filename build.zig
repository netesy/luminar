const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard target and optimization options
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Create the main executable
    const exe = b.addExecutable(.{
        .name = "luminar",
        .target = target,
        .optimize = optimize,
    });
    exe.subsystem = .Console;

    // Define C++ compiler flags
    const cpp_flags = &[_][]const u8{
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-g", // Debug symbols
        "-O2", // Optimization (you can adjust based on build type)
    };

    // Core source files (matching your CMake structure)
    const source_files = [_][]const u8{
        "src/main.cpp",
        "src/debugger.cpp",
        "src/repl.cpp",
        "src/scanner.cpp",
        "src/ast.cpp",
        "src/vm.cpp",
        "src/memory.cpp",
        "src/function.cpp",
        "src/backends/jit.cpp",
        "src/backends/codegen.cpp",
        "src/backends/register.cpp",
        "src/backends/stack.cpp",
        "src/backends/yasm.cpp",
        "src/parser/packrat.cpp",
        "src/parser/pratt.cpp",
    };

    // Add all source files to the executable
    for (source_files) |file_path| {
        exe.addCSourceFile(.{
            .file = .{ .path = file_path },
            .flags = cpp_flags,
        });
    }

    // Link against system libraries
    exe.linkLibC();
    exe.linkLibCpp();

    // Add include directories
    const include_paths = [_][]const u8{
        "src",
        "src/backends",
        "src/parser",
        "test", // For test files
    };

    for (include_paths) |path| {
        exe.addIncludePath(.{ .path = path });
    }

    // Optional: Add GCC JIT library support (uncomment if needed)
    // This matches your commented CMake section
    // exe.linkSystemLibrary("gccjit");

    // Install the executable
    b.installArtifact(exe);

    // Create run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    
    // Forward command line arguments to the executable
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the luminar application");
    run_step.dependOn(&run_cmd.step);

    // Test build disabled
    // // Create test executable for C++ tests
    // const test_exe = b.addExecutable(.{
    //     .name = "luminar-test",
    //     .target = target,
    //     .optimize = optimize,
    // });
    // test_exe.subsystem = .Console;

    // // Test source files
    // const test_source_files = [_][]const u8{
    //     "test/tst_parser.cpp",
    //     "test/tst_scanner.cpp",
    //     // Add other test files as needed
    // };

    // // Add test-specific source files
    // for (test_source_files) |file_path| {
    //     test_exe.addCSourceFile(.{
    //         .file = .{ .path = file_path },
    //         .flags = cpp_flags,
    //     });
    // }

    // // Add core source files needed for tests (excluding main.cpp to avoid multiple main functions)
    // const test_core_files = [_][]const u8{
    //     "src/debugger.cpp",
    //     "src/repl.cpp",
    //     "src/scanner.cpp",
    //     "src/ast.cpp",
    //     "src/vm.cpp",
    //     "src/memory.cpp",
    //     "src/function.cpp",
    //     "src/backends/jit.cpp",
    //     "src/backends/codegen.cpp",
    //     "src/backends/register.cpp",
    //     "src/backends/stack.cpp",
    //     "src/backends/yasm.cpp",
    //     "src/parser/packrat.cpp",
    //     "src/parser/pratt.cpp",
    // };

    // for (test_core_files) |file_path| {
    //     test_exe.addCSourceFile(.{
    //         .file = .{ .path = file_path },
    //         .flags = cpp_flags,
    //     });
    // }

    // // Link test executable
    // test_exe.linkLibC();
    // test_exe.linkLibCpp();

    // // Add include paths for tests
    // for (include_paths) |path| {
    //     test_exe.addIncludePath(.{ .path = path });
    // }

    // // Install test executable
    // b.installArtifact(test_exe);

    // // Create test run step
    // const test_run_cmd = b.addRunArtifact(test_exe);
    // test_run_cmd.step.dependOn(b.getInstallStep());

    // const test_step = b.step("test", "Run C++ unit tests");
    // test_step.dependOn(&test_run_cmd.step);



    const install_docs = b.addInstallDirectory(.{
        .source_dir = .{ .path = "doc" },
        .install_dir = .prefix,
        .install_subdir = "share/doc/luminar",
    });

    const doc_step = b.step("install-docs", "Install documentation files");
    doc_step.dependOn(&install_docs.step);

    // Create a clean step for convenience
    const clean_step = b.step("clean", "Clean build artifacts");
    clean_step.dependOn(&b.addRemoveDirTree(b.install_path).step);
}