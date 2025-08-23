CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Detect libgccjit
GCCJIT_LIB := $(shell ldconfig -p 2>/dev/null | grep libgccjit.so | awk '{print $$4}' | head -n1)
LDFLAGS := 
ifneq ($(GCCJIT_LIB),)
    LDFLAGS += -lgccjit
endif

TARGET := luminar

SRC := \
    src/main.cpp \
    src/debugger.cpp \
    src/repl.cpp \
    src/scanner.cpp \
    src/ast.cpp \
    src/vm.cpp \
    src/backends/jit.cpp \
    src/backends/codegen.cpp \
    src/backends/register.cpp \
    src/backends/stack.cpp \
    src/backends/yasm.cpp \
    src/parser/packrat.cpp \
    src/parser/pratt.cpp \
    src/memory.cpp \
    test/tst_parser.cpp \
    test/tst_scanner.cpp

OBJ := $(SRC:.cpp=.o)

all: check-libgccjit $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure libgccjit is installed
check-libgccjit:
	@if [ -z "$(GCCJIT_LIB)" ]; then \
		echo "libgccjit not found. Installing..."; \
		if command -v apt-get >/dev/null 2>&1; then \
			sudo apt-get update && sudo apt-get install -y libgccjit-10-dev || sudo apt-get install -y libgccjit-12-dev; \
		elif command -v dnf >/dev/null 2>&1; then \
			sudo dnf install -y libgccjit libgccjit-devel; \
		elif command -v yum >/dev/null 2>&1; then \
			sudo yum install -y libgccjit libgccjit-devel; \
		elif command -v pacman >/dev/null 2>&1; then \
			sudo pacman -Sy --noconfirm libgccjit; \
		else \
			echo "No supported package manager found. Please install libgccjit manually."; \
			exit 1; \
		fi \
	fi

# Install locations
PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
DOCDIR := $(PREFIX)/share/doc/$(TARGET)

install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)
	install -d $(DOCDIR)
	install -m 644 doc/readme.md doc/issues.md doc/features.md doc/design.md doc/implementation.md doc/zen.md $(DOCDIR)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean install check-libgccjit
