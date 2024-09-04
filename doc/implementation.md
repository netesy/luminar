# Luminar Language Implementation Document

## Table of Contents

1. [Overview](#overview)
2. [Syntax and Semantics](#syntax-and-semantics)
   - [Comments](#comments)
   - [Basic Syntax](#basic-syntax)
3. [Data Types](#data-types)
   - [Basic Data Types](#basic-data-types)
   - [Composite Types](#composite-types)
4. [Operators](#operators)
5. [Control Flow Structures](#control-flow-structures)
6. [Error Handling](#error-handling)
7. [Functions](#functions)
8. [Classes](#classes)
9. [Modules](#modules)
10. [Pattern Matching](#pattern-matching)
11. [Concurrency and Parallelism](#concurrency-and-parallelism)
12. [Memory Management](#memory-management)
13. [Do's and Don'ts](#dos-and-donts)

## Overview

Luminar is a statically typed language designed to balance readability, efficiency, and flexibility. It supports modern features, including advanced memory management, modularity, and concurrency. The language follows principles to ensure safe and efficient development across a range of applications from systems programming to high-level application development.

## Syntax and Semantics

### Comments

- **Use:** To explain and document code.
- **Syntax:**
  ```luminar
  // Single-line comment
  /* Multi-line comment */
  /** Documentation comment */
  ```

- **Do's:**
  - Use comments to clarify complex logic or intent.
  - Document functions, classes, and modules with documentation comments.

- **Don'ts:**
  - Avoid redundant comments that state the obvious.
  - Don't use comments as a substitute for clear code.

### Basic Syntax

- **Arithmetic Operations:**
  ```luminar
  print(1 + 2);       // Addition
  print(10 / 5);      // Division
  print(3 * 4);       // Multiplication
  ```

- **Logical Operators:**
  ```luminar
  print(true && false); // Logical AND
  print(true || false); // Logical OR
  ```

- **String Concatenation:**
  ```luminar
  print("Hello" + " " + "World"); // Output: Hello World
  ```

- **Do's:**
  - Write expressions clearly to improve readability.
  - Use appropriate operators for clarity and intent.

- **Don'ts:**
  - Avoid using complex expressions in a single line.
  - Don't mix logical and arithmetic operators without parentheses.

## Data Types

### Basic Data Types

- **Nil:** Represents the absence of a value.
- **Bool:** Boolean values (`true`, `false`).
- **Integers:** `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`.
- **Floating-Point Numbers:** `f32`, `f64`.
- **String:** Represents a sequence of characters (`str`).

- **Do's:**
  - Choose the smallest integer type that fits your data to optimize memory usage.
  - Use `f32` for less precision and `f64` for more precision.

- **Don'ts:**
  - Avoid using floating-point types for financial or precise calculations.
  - Don't mix data types in arithmetic operations without explicit conversions.

### Composite Types

- **List:** An ordered collection of elements.
- **Dict:** A collection of key-value pairs.
- **Enum:** A type with a set of predefined values.
- **Function:** Defines parameter and return types.
- **Sum:** A variant type allowing different types.
- **Union:** Represents a value that can be one of several types.
- **UserDefined:** Custom structured types.
- **Optional:** Represents an optional value (`?`).

- **Do's:**
  - Use composite types to group related data together.
  - Leverage optional types to handle cases where a value may be absent.

- **Don'ts:**
  - Avoid overcomplicating data structures with unnecessary nesting.
  - Don't use complex composite types without clear documentation.

## Operators

- **Arithmetic:** `+`, `-`, `*`, `/`, `%`.
- **Comparison:** `==`, `!=`, `<`, `>`, `<=`, `>=`.
- **Logical:** `&&`, `||`, `!`.
- **String Concatenation:** `+`.

- **Do's:**
  - Use comparison operators for conditional checks.
  - Employ logical operators to combine conditions.

- **Don'ts:**
  - Avoid using multiple operators in a single expression without parentheses.
  - Don't rely on operator precedence without clarity.

## Control Flow Structures

### For Loop Over a Range

```luminar
for (i in 1..10) {
  print(i);
}
```

### For Loop Over a List

```luminar
var my_list: list<int> = (1, 2, 3);
for (item in my_list) {
  print(item);
}
```

### While Loop

```luminar
var i: int = 0;
while (i < 5) {
  print(i);
  i += 1;
}
```

- **Do's:**
  - Use loops for iterative tasks and clear logic.
  - Ensure loops have a terminating condition to avoid infinite loops.

- **Don'ts:**
  - Avoid using loops for operations that can be done with built-in functions or comprehensions.
  - Don't write complex loop conditions without comments.

## Error Handling

### Error Handling in Functions

```luminar
fn divide(a: i32, b: i32) -> (i32, i32) {
  if b == 0 {
    return (0, -1); // Error code for division by zero
  } else {
    return (a / b, 0); // Success
  }
}

// Check error code
var (result, err) = divide(10, 0);
if err == 0 {
  print(result);
} else {
  print("Error: division by zero");
}
```

- **Do's:**
  - Handle errors explicitly and provide meaningful error codes.
  - Validate inputs and return appropriate error messages or codes.

- **Don'ts:**
  - Avoid ignoring error codes or results from functions.
  - Don't use generic error handling; be specific about the type of error.

## Functions

### Function Definition

```luminar
fn add(a: i32, b: i32) -> i32 {
  return a + b;
}
```

### Function with Optional Parameter

```luminar
fn greet(name: str, date: str? = None): str {
  var message: str = format("Hello {name}");
  if date.is_some() {
    message = format("{message}, today's date is {date.unwrap()}");
  }
  return message;
}
```

- **Do's:**
  - Define functions with clear parameter and return types.
  - Use optional parameters for flexibility.

- **Don'ts:**
  - Avoid functions with excessive parameters; refactor if necessary.
  - Don't make functions too complex; keep them focused on a single task.

## Classes

### Class Definition

```luminar
class Person {
  var name: str;
  var age: int;

  Person(name: str, age: int) {
    self.name = name;
    self.age = age;
  }

  fn display() {
    print("Name: {self.name}");
    print("Age: {self.age}");
  }
}

var person1: Person = Person("Alice", 30);
person1.display();
```

- **Do's:**
  - Use classes to encapsulate data and behaviors.
  - Keep class methods focused on the class's purpose.

- **Don'ts:**
  - Avoid deeply nested class hierarchies.
  - Don't use classes for simple data structures; prefer structs or simpler types.

## Modules

### Defining and Using Modules

```luminar
module Sample {
  fn greet() {
    print("Hello from Sample module");
  }
}
```

- **Do's:**
  - Organize code into modules for better maintainability.
  - Import only necessary modules to avoid dependencies.

- **Don'ts:**
  - Avoid circular dependencies between modules.
  - Don't overload modules with unrelated functionalities.

## Pattern Matching

### Example of Pattern Matching

```luminar
fn match_example(value: Any): void {
  match value {
    int => print("Integer value"),
    str => print("String value"),
    list<T> => print("List value"), // Generic type placeholder
    dict<K, V> => print("Dictionary value"), // Generic type placeholders
    _ => print("Unknown type"),
  }
}
```

- **Do's:**
  - Use pattern matching to handle different data types and structures.
  - Be exhaustive in matching to cover all possible cases.

- **Don'ts:**
  - Avoid using pattern matching without handling all possible variants.
  - Don't use overly complex patterns that reduce readability.

## Concurrency and Parallelism

### Concurrency Example

```luminar
fn concurrent_task(id: int) {
  print("Concurrent task {id} running");
}

concurrent(tasks, cores=Auto) {
  for (i in 1..3) {
    run_task(i);
  }
}
```

### Parallelism Example

```luminar
fn parallel_task(id: int) {
  print("Parallel task {id} running");
}

parallel(tasks) {
  for (i in 1..3) {
    spawn_task(i);


  }
}
```

- **Do's:**
  - Use concurrency for tasks that can run independently.
  - Utilize parallelism for tasks that benefit from simultaneous execution.

- **Don'ts:**
  - Avoid overusing concurrency and parallelism, which can lead to complexity and bugs.
  - Don't assume tasks will always run in parallel without proper synchronization.

## Memory Management

Luminar integrates linear-first and reference-first memory management approaches with manual management in unsafe blocks.

### Linear-First Approach

- **Regions:** Automatic memory management within regions, with deallocation upon scope exit.
- **Automatic Management:** Handles memory automatically to reduce risks of leaks.

### Reference-First Approach

- **References (`ref`):** Mutable by default, simplifying reference handling.
- **Linear Conversion (`lin`):** Explicit conversion from references to linear types.

### Unsafe Mode

- **Manual Management:** Use raw pointers (`*u8`) for fine-grained control over memory in `unsafe` blocks.

- **Do's:**
  - Prefer automatic memory management for general use cases.
  - Use unsafe mode only for performance-critical or low-level operations.

- **Don'ts:**
  - Avoid manual memory management unless necessary.
  - Don't mix safe and unsafe code without careful consideration.

## Do's and Don'ts

### Do's

- **Write Clear Code:** Ensure that code is easy to read and understand.
- **Optimize When Needed:** Focus on performance while maintaining code clarity.
- **Use Language Features:** Leverage Luminar's features to their full extent.
- **Document Code:** Provide comments and documentation for clarity.
- **Handle Errors:** Explicitly handle errors and edge cases.

### Don'ts

- **Avoid Complexity:** Don't make code unnecessarily complex or obscure.
- **Neglect Safety:** Never compromise on safety for performance without proper checks.
- **Overuse Unsafe Code:** Limit the use of `unsafe` blocks to essential areas.
- **Ignore Conventions:** Follow uniform conventions and patterns throughout the codebase.

---

This document provides a structured overview of Luminar's features and guidelines, aligning with the Zen of Luminar. It aims to support developers in creating efficient, maintainable, and safe code using Luminar.