# FAMILIAR features it should have (the most important ones):

Functional Programming, but based on data-first piping.

Immutability, but w/ opportunistic in-place mutation.

Gradually typed: dynamic for development, static for production (w/ fully sound type inference).

Concurrency via goroutines and (the async) unbounded buffered channels.

Ecosystem: interoperable with existing languages.

Transpiles to JS and/or compiles to WASM.

Adaptive runtime.

## ESOTERIC features it should have (the most important ones):

Crucial evolvability / backward- and forward-compatibility.

Content-addressable code.

Transparent upgrades without any breaking changes.

Data First Functional Programming w/ SVO style syntax: subject.verb(object)

Interpreted, for development. But compiled, incrementally, for production.

Interactive: facilitates an IDE-plugin (VS Code) that shows the contents of data structures while coding. Enable REPL'ing into a live system.

Aggressively parallelizable and concurrent. Inspired by Verilog and Golang.

Scales transparently from single CPU to multi-core to distributed services, without the language necessitating a refactoring of the code.

### OTHER features it should have (a short-list of some familiar ones):

Eager evaluation (strict, call-by-value), Strong & Static Typing with Fully Sound Type Inference, Generics, Algebraic Data Types (ADT), No Null, No Exceptions by default, No undefined behavior, No Garbage Collector (GC), Async via a blocking/sync interface but non-blocking I/O, Reliable Package Management, Tree Shakeable, Serializable, Helpful Error Messages, Good Interoperability, and First-Class Functions and Higher-Order Functions (HOC), of course (since FP).