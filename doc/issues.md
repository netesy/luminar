# Issues With Luminar

```json
    while(0<3){
       print("infinite loop");
       } 
''' //success
== ```  var m = 0; while(m<3){print(m); m=m+1;}```== //error the presence of the first variable stops the parsing
[x] Not functional

issues with variables when starting loops structures

while(12>3){print(true);}
var m = 12; if(m>50){print(1);}else{print(0);}





Sure, let's incorporate string interpolation into the Luminar README. Here's how the updated section would look:

---

## Luminar Programming Language

**Luminar** is a statically typed programming language designed for readability, efficiency, and modern features. It is intended to be interpreted for development and compiled for release. This README provides a structured overview to help you learn Luminar effectively.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Syntax Overview](#syntax-overview)
   - [Comments](#comments)
   - [Data Types](#data-types)
   - [Operators](#operators)
   - [Control Flow Statements](#control-flow-statements)
   - [Error Handling](#error-handling)
   - [Functions](#functions)
   - [Classes](#classes)
   - [Modules](#modules)
   - [Pattern Matching](#pattern-matching)
   - [String Interpolation](#string-interpolation)
3. [Key Features](#key-features)
4. [Basic Syntax](#basic-syntax)
   - [Arithmetic Operations](#arithmetic-operations)
   - [Logical Operators](#logical-operators)
   - [Unary Plus and Minus](#unary-plus-and-minus)
   - [String Concatenation](#string-concatenation)
   - [String Interpolation](#string-interpolation-1)
5. [Pointer Handling and Memory Allocation](#pointer-handling-and-memory-allocation)
   - [Allocating and Using Pointers](#allocating-and-using-pointers)
   - [Functions](#functions-1)
   - [Error Handling](#error-handling-1)
   - [Detailed Memory Management Example](#detailed-memory-management-example)
   - [Memory Management: Safe vs. Unsafe](#memory-management-safe-vs-unsafe)
6. [Luminar Type System](#luminar-type-system)
   - [Basic Data Types](#basic-data-types)
   - [Composite Types](#composite-types)
7. [Example Code Snippets](#example-code-snippets)
8. [Contributing](#contributing)

## Getting Started

Luminar is under development. Installation instructions will be provided once the language becomes publicly available.

## Syntax Overview

### Comments
- **Single-line comments:** `// This is a comment`
- **Multi-line comments:** `/* This is a multi-line comment */`
- **Documentation comments:** `/// This is a documentation comment`

### Data Types
#### Basic Data Types
- **Nil:** Represents the absence of a value.
- **Bool:** Boolean type.
- **Integers:**
  - `int`: Platform-dependent signed integer
  - `uint`: Platform-dependent unsigned integer
  - `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`
- **Floating-Point Numbers:**
  - `f32`, `f64`
  - `float`: Platform-dependent floating point
- **String:** Represents a sequence of characters.
  - `str`

### Operators
- **Arithmetic Operators:** `+`, `-`, `*`, `/`, `%`
- **Comparison Operators:** `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical Operators:** `&&` (and), `||` (or), `!` (not)
- **String Concatenation:** `+`

### Control Flow Statements
- `if`, `elif`, `else` statements
- `for` loops (range-based and list-based)
- `while` loops

### Error Handling
- Functions can return error types.

### Functions
- Define functions with optional parameters and return types using the `fn` keyword.

### Classes
- Define classes with constructors and methods using the `class` keyword.

### Modules
- Organize code into modules using the `module` keyword.

### Pattern Matching
- Use `match` keyword for pattern matching.

### String Interpolation
- **Syntax:** `print("Welcome {name}");` where `{name}` is replaced with the value of the `name` variable.

## Key Features
- **Subtyping and Row Polymorphism:** Flexible function arguments and return types.
- **Inline Assembly (Restricted):** Low-level control with safety considerations.
- **Unsafe (Controlled):** Access to advanced features with careful use.

## Basic Syntax

### Arithmetic Operations

```luminar
print(1 + 2 + (3 * 4) - (10 / 5));  // Output: 11
```

### Logical Operators

```luminar
print((12 > 3) && (12 > 13));    // false (AND requires both conditions true)
print((12 > 3) || (12 > 13));    // true (OR requires at least one condition true)
```

### Unary Plus and Minus

```luminar
print(+10);  // 10 (unary plus is a no-op)
print(-5);   // -5
```

### String Concatenation

```luminar
print("Hello" + " " + "World");  // Output: Hello World
```

### String Interpolation

```luminar
let name: str = "Alice";
print("Welcome {name}");  // Output: Welcome Alice
```

## Pointer Handling and Memory Allocation

### Allocating and Using Pointers

```luminar
// Allocating memory for an integer and assigning a value
let p: *i32 = allocate(i32);
*p = 42;

// Allocating memory for an array of integers and initializing values
let arr: *i32 = allocate(i32, 10);
for i in 0..10 {
    arr[i] = i;  // Simplified array access using index notation
}

// Printing the values
println(*p);  // Accessing the value of the pointer
for i in 0..10 {
    println(arr[i]);  // Accessing array elements
}

// Freeing allocated memory
deallocate(p);
deallocate(arr);
```

### Functions

```luminar
// Function returning an integer
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

// Function taking a string reference (pointer) and returning nil
fn greet(name: *string) -> nil {
    println("Hello, " + *name);
}
```

### Error Handling

```luminar
// Function that returns a result type with error code
fn divide(a: i32, b: i32) -> (i32, i32) {  // (result, error code)
    if b == 0 {
        return (0, -1);  // Error: division by zero
    } else {
        return (a / b, 0);  // Success
    }
}

// Using the function with error checking
let (result, err) = divide(10, 2);
if err == 0 {
    println("Result: " + result);
} else {
    println("Error: division by zero");
}
```

### Detailed Memory Management Example

```luminar
fn main() {
    // Allocating memory for an integer and assigning a value
    let p: *i32 = allocate(i32);
    *p = 100;

    // Allocating memory for an array of integers and initializing values
    let arr: *i32 = allocate(i32, 5);
    for i in 0..5 {
        arr[i] = i * 2;  // Simplified array access using index notation
    }

    // Printing the values
    println("Single integer: " + *p);
    for i in 0..5 {
        println("Array element " + i + ": " + arr[i]);
    }

    // Freeing allocated memory
    deallocate(p);
    deallocate(arr);
}
```

### Memory Management: Safe vs. Unsafe

```luminar
fn main() {
    // Using manual memory management within an unsafe block
    unsafe {
        let p: *i32 = allocate(i32);
        *p = 100;
        println("Unsafe single integer: " + *p);
        deallocate(p);
    }

    // Using automatic garbage collection
    let gc_string: string = "This is managed by GC";
    println(gc_string);
}
```

## Luminar Type System

### Basic Data Types
- **Nil:** Represents the absence of a value.
- **Bool:** Boolean type.
- **Integers:**
  - `int`: Platform-dependent signed integer
  - `uint`: Platform-dependent unsigned integer
  - `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`
- **Floating-Point Numbers:**
  - `f32`, `f64`
  - `float`: Platform-dependent floating point
- **String:** Represents a sequence of characters.
  - `str`

### Composite Types
- **List:** A list of elements of a specific type.
- **Dict:** A dictionary with keys and values of specific types.
- **Enum:** An enumeration of predefined values.
- **Function:** A function type with parameter types and a return type.
- **Sum:** A sum type (tagged union or variant) that can hold any one of several specified types.
- **Union:** A union type that represents a value that can be one of several types.
- **UserDefined:** A user-defined type allowing custom structured types.
- **Optional:** Represented by `?`, allowing for optional values.

## Example Code Snippets

Refer to the provided code examples for demonstrations of various functionalities.

## Contributing

Luminar is an open-source project. Contributions to its





### Allocating and Using Pointers

```luminar
// Allocating memory for an integer and assigning a value
let p: *i32 = allocate(i32);
*p = 42;

// Allocating memory for an array of integers and initializing values
let arr: *i32 = allocate(i32, 10);
for i in 0..10 {
    arr[i] = i;  // Simplified array access using index notation
}

// Printing the values
println(*p);  // Accessing the value of the pointer
for i in 0..10 {
    println(arr[i]);  // Accessing array elements
}

// Freeing allocated memory
deallocate(p);
deallocate(arr);
```

### Functions

```luminar
// Function returning an integer
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

// Function taking a string reference (pointer) and returning nil
fn greet(name: *string) -> nil {
    println("Hello, " + *name);
}
```

### Error Handling

```luminar
// Function that returns a result type with error code
fn divide(a: i32, b: i32) -> (i32, i32) {  // (result, error code)
    if b == 0 {
        return (0, -1);  // Error: division by zero
    } else {
        return (a / b, 0);  // Success
    }
}

// Using the function with error checking
var (result, err) = divide(10, 2);
if err == 0 {
    println("Result: " + result);
} else {
    println("Error: division by zero");
}
```
