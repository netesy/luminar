cmake_minimum_required(VERSION 3.5)

project(luminar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(luminar main.cpp
    debugger.cpp debugger.hh
    parser.cpp parser.hh
    repl.cpp repl.hh
    scanner.cpp scanner.hh
    syntax.cpp syntax.hh
    sample.lm sample_new.lm
    readme.md issues.md features.md
    ast.hh ast.cpp
    vm.hh vm.cpp vm-old.cx
    main.cpp
    opcodes.hh
    types.hh
    helper.hh
    variable.hh
    precedence.hh
    instructions.hh
    backends/jit.hh backends/jit.cpp
    backends/codegen.hh backends/codegen.cpp
    backends/register.hh backends/register.cpp
    backends/stack.hh backends/stack.cpp
    backends/import.hh
    backends/backend.hh
    test/tst_parser.cpp
    test/tst_scanner.cpp
    token.hh
    backends/yasm.hh backends/yasm.cpp)

include(GNUInstallDirs)
install(TARGETS luminar
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
