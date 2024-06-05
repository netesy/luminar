cmake_minimum_required(VERSION 3.5)

project(luminar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(luminar main.cpp
    debugger.cpp debugger.hh
    main.cpp 
    opcodes.hh 
    parser.cpp 
    parser.hh 
    repl.cpp repl.hh 
    scanner.cpp scanner.hh
    syntax.cpp syntax.hh
    types.hh
    sample.lm sample_new.lm
    ast.hh ast.cpp
    vm.hh vm.cpp
    symbol.hh
    helper.hh instructions.hh 
    backends/backend.hh 
    backends/codegen.hh 
    backends/codegen.cpp 
    backends/import.hh 
    backends/register.hh 
    backends/register.cpp 
    backends/stack.hh 
    backends/stack.cpp  )

include(GNUInstallDirs)
install(TARGETS luminar
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
