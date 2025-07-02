cmake_minimum_required(VERSION 3.10)

project(luminar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(luminar src/main.cpp
    src/debugger.cpp src/debugger.hh

    src/repl.cpp src/repl.hh
    src/scanner.cpp src/scanner.hh
    src/ast.hh src/ast.cpp
    src/vm.hh src/vm.cpp
    src/main.cpp
    src/opcodes.hh
    src/types.hh
    src/helper.hh
    src/variable.hh
    src/precedence.hh
    src/instructions.hh
    src/backends/jit.hh src/backends/jit.cpp
    src/backends/codegen.hh src/backends/codegen.cpp
    src/backends/register.hh src/backends/register.cpp
    src/backends/stack.hh src/backends/stack.cpp
    src/backends/import.hh
    src/backends/backend.hh
    src/token.hh
    src/backends/yasm.hh src/backends/yasm.cpp
    src/scope.hh
     src/function.hh src/function.cpp src/function_types.hh src/builtin_function.hh
    src/parser/packrat.hh src/parser/packrat.cpp
    src/parser/pratt.hh src/parser/pratt.cpp
    src/parser/pratt_packrat.hh src/parser/pratt_packrat.cpp
    src/parser/algorithm.hh
    src/memory.hh src/memory.cpp
    test/tst_parser.cpp
    test/tst_scanner.cpp
    sample/sample.lm
    src/optimizer.hh
    src/value.hh
    src/classes.hh
    src/tutorial.hh
    src/memory_analyzer.hh

)

find_library(GCCJIT_LIBRARY NAMES libgccjit.so libgccjit.dll)

# # Use execute_process to get the correct include path dynamically
# execute_process(
#     COMMAND gcc -print-file-name=include
#     OUTPUT_VARIABLE GCC_INCLUDE_DIR
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# include_directories(${GCC_INCLUDE_DIR}/jit)

# target_link_libraries(luminar ${GCCJIT_LIBRARY})

include(GNUInstallDirs)
install(TARGETS luminar
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install the documentation files
install(FILES
    doc/readme.md
    doc/issues.md
    doc/features.md
    doc/design.md
    doc/implementation.md
    doc/zen.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
