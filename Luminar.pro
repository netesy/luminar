# Luminar.pro - Qt Creator project file for Luminar

QT += core gui widgets

CONFIG += c++17 console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = luminar
TEMPLATE = app

SOURCES += \
    src/backends/codegen.cpp \
    src/backends/jit.cpp \
    src/backends/register.cpp \
    src/backends/stack.cpp \
    src/backends/yasm.cpp \
    src/parser/packrat.cpp \
    src/parser/pratt.cpp \
    src/ast.cpp \
    src/debugger.cpp \
    src/main.cpp \
    src/memory.cpp \
    src/repl.cpp \
    src/scanner.cpp \
    src/vm.cpp

HEADERS += \
    src/backends/backend.hh \
    src/backends/codegen.hh \
    src/backends/import.hh \
    src/backends/jit.hh \
    src/backends/register.hh \
    src/backends/stack.hh \
    src/backends/yasm.hh \
    src/parser/algorithm.hh \
    src/parser/packrat.hh \
    src/parser/pratt.hh \
    src/ast.hh \
    src/builtin_function.hh \
    src/classes.hh \
    src/debugger.hh \
    src/function.hh \
    src/helper.hh \
    src/instructions.hh \
    src/memory.hh \
    src/memory_analyzer.hh \
    src/opcodes.hh \
    src/optimizer.hh \
    src/precedence.hh \
    src/repl.hh \
    src/scanner.hh \
    src/scope.hh \
    src/token.hh \
    src/tutorial.hh \
    src/types.hh \
    src/value.hh \
    src/variable.hh \
    src/vm.hh

INCLUDEPATH += src/

# Uncomment these lines if you need to link against libgccjit
# LIBS += -lgccjit
# INCLUDEPATH += /usr/include/gcc

# Installation paths
target.path = /usr/local/bin
INSTALLS += target
