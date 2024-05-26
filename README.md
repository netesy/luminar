# Luminar Programming Language

**Luminar** is a new, statically typed programming language designed for readability, efficiency, and expressiveness.

This README provides a basic overview of the language features.

## Getting Started

Luminar is currently under development. We'll provide installation instructions when the language becomes publicly available.

### Syntax Overview

* Single-line comments: `// This is a comment`
* Multi-line comments: `/* This is a multi-line comment */`
* Documentation comments: `/// This is a documentation comment`

**Supported Data Types:**

* `int`: Integers
* `str`: Strings
* `bool`: Booleans
* `list<T>`: Lists with elements of type `T` (generic type, planned)
* `dict<K, V>`: Dictionaries with keys of type `K` and values of type `V` (generic types, planned)
* `?`: optional

**Supported Operators:**

* Arithmetic operators: `+`, `-`, `*`,`%`, `/`
* Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
* Logical operators: `&&` (and), `||` (or), `!` (not)
* String concatenation: `+`


**Control Flow Statements:**

* `if` statements
* `else if` statements
* `else` statements
* `for` loops (range-based and list-based)
* `while` loops

**Error Handling:**

* Functions can return error types.

**Functions:**

* Define functions with optional parameters and return types.
* Use `fn` keyword to declare functions.

**Classes:**

* Define classes with constructors and methods.
* Use `class` keyword to define classes.

**Modules:**

* Organize code into modules for better structure.
* Use `module` keyword to define modules.

**Pattern Matching:**

* Use pattern matching for conditional logic based on data types.
* Use `match` keyword for pattern matching.

**Experimental Features:**

* Dependent/Refinement types (under development)
* File I/O (under development)
* Concurrency and Asynchronicity (planned for future versions)
* Generics (planned for future versions)

**Examples:**

The README includes several code examples demonstrating various language features.

**Please note:**

* Luminar is under development, and some features like dependent types and file I/O are experimental and subject to change.
* Concurrency and Asynchronicity are planned for future versions.
* Generics are also planned for future versions.

We welcome your feedback and contributions!

#### Luminar Programming Languages

**Luminar** is a new, statically typed programming language designed for readability, efficiency, and modern features. This README provides a brief overview of the language's syntax and functionalities.

**Getting Started:**

Luminar is currently under development, but you can explore its core features through this documentation.

**Comments:**

* Single-line comments: `//`
* Multi-line comments: `/* */`
* Documentation comments: `///`

**Key Features:**

* **Subtyping and Row Polymorphism:** Enables flexible function arguments and return types.
* **Inline Assembly (Restricted):** Offers low-level control with safety considerations.
* **Unsafe (Controlled):** Grants access to advanced features but requires careful use.

**Basic Syntax:**

* **Arithmetic Operations:** Standard mathematical operators (+, -, *, /).
* **Logical Operators:** `and`, `or`, `not`.
* **Comparison Operators:** `==`, `!=`, `<`, `>`, `<=`, `>=`.
* **Unary Plus and Minus:** `+` (unary positive), `-` (unary negative).
* **String Concatenation:** `+` for string concatenation.

**Functions:**

* Define functions with `fn` keyword.
* Optional parameters and default values.
* String formatting with `format!`.
* Pattern matching for optional parameters using `Some` and `None`.

**Control Flow Structures:**

* **For Loops:** Iterate over ranges or lists.
* **While Loops:** Conditional execution.

**Error Handling:**

* Dedicated error return type (`error`).

**List and Dictionary Support:**

* Lists (`list<T>`) for collections of elements.
* Dictionaries (`dict<K, V>`) for key-value pairs.

**Classes:**

* Define classes with `class` keyword.
* Constructors for object initialization.
* Methods for defining class functionalities.

**Modules:**

* Organize code into modules using `module`.

**Dependent/Refinement Types (Experimental):**

* Explore advanced type checking capabilities (under development).

**Concurrency and Asynchronicity (Planned):**

* Future versions will introduce features for parallel and asynchronous programming.

**Pattern Matching:**

* Perform conditional logic based on value types using `match`.

**Generics (Planned):**

* Support for generic functions and types planned for future releases.

**Example Code Snippets:**

Refer to the provided code examples within the README for demonstrations of various functionalities.

**Contributing:**

Luminar is an open-source project. We welcome contributions to its development. Stay tuned for further updates and documentation as the language evolves.
