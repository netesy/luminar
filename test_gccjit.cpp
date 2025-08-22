#include <libgccjit.h>
#include <iostream>

int main() {
    gcc_jit_context *ctxt = gcc_jit_context_acquire();
    if (!ctxt) {
        std::cerr << "Failed to acquire JIT context" << std::endl;
        return 1;
    }
    gcc_jit_context_release(ctxt);
    std::cout << "Successfully linked against libgccjit" << std::endl;
    return 0;
}
