# Luminar Language Design

> **STATUS:** Up-to-date as of 21-Aug-2024.


## Table of Contents

- [Introduction](#introduction)
    - [This Document is Provisional](#this-document-is-provisional)
    - [Tour of the Basics](#tour-of-the-basics)
- [Code and Comments](#code-and-comments)
- [Build Modes](#build-modes)
- [Types are Values](#types-are-values)
    - [Values Usable as Types](#values-usable-as-types)
- [Primitive Types](#primitive-types)
    - [`bool`](#bool)
    - [Integer Types](#integer-types)
        - [Integer Literals](#integer-literals)
    - [Fixed-Point Types](#fixed-point-types)
        - [Fixed-Point Literals](#fixed-point-literals)
    - [String Types](#string-types)
        - [String Literals](#string-literals)
- [Values, Objects, and Expressions](#values-objects-and-expressions)
    - [Expression Categories](#expression-categories)
    - [Expression Phases](#expression-phases)
- [Composite Types](#composite-types)
    - [Tuples](#tuples)
    - [Struct Types](#struct-types)
    - [Pointer Types](#pointer-types)
    - [Arrays and Slices](#arrays-and-slices)
- [Expressions](#expressions)
- [Declarations, Definitions, and Scopes](#declarations-definitions-and-scopes)
- [Patterns](#patterns)
    - [Binding Patterns](#binding-patterns)
    - [Destructuring Patterns](#destructuring-patterns)
    - [Refutable Patterns](#refutable-patterns)
- [Name-Binding Declarations](#name-binding-declarations)
    - [Constant `let` Declarations](#constant-let-declarations)
    - [Variable `var` Declarations](#variable-var-declarations)
    - [`auto`](#auto)
    - [Global Constants and Variables](#global-constants-and-variables)
- [Functions](#functions)
    - [Parameters](#parameters)
    - [`auto` Return Type](#auto-return-type)
    - [Blocks and Statements](#blocks-and-statements)
    - [Control Flow](#control-flow)
        - [`if` and `else`](#if-and-else)
        - [Loops](#loops)
            - [`while`](#while)
            - [`for`](#for)
            - [`break`](#break)
            - [`continue`](#continue)
        - [`return`](#return)
            - [`returned var`](#returned-var)
        - [`match`](#match)
- [User-Defined Types](#user-defined-types)
    - [Classes](#classes)
        - [Assignment](#assignment)
        - [Class Functions and Factory Functions](#class-functions-and-factory-functions)
        - [Methods](#methods)
        - [Inheritance](#inheritance)
        - [Access Control](#access-control)
        - [Destructors](#destructors)
        - [`const`](#const)
        - [Unformed State](#unformed-state)
        - [Move](#move)
        - [Mixins](#mixins)
    - [Choice Types](#choice-types)
- [Names](#names)
    - [Files, Libraries, Packages](#files-libraries-packages)
    - [Package Declaration](#package-declaration)
    - [Imports](#imports)
        - [Same-Package Imports](#same-package-imports)
        - [Cross-Package Imports](#cross-package-imports)
    - [Name Visibility](#name-visibility)
    - [Package Scope](#package-scope)
    - [Namespaces](#namespaces)
    - [Naming Conventions](#naming-conventions)
    - [Aliases](#aliases)
    - [Name Lookup](#name-lookup)
        - [Name Lookup for Common Types](#name-lookup-for-common-types)
- [Generics](#generics)
    - [Checked and Template Parameters](#checked-and-template-parameters)
    - [Interfaces and Implementations](#interfaces-and-implementations)
    - [Combining Constraints](#combining-constraints)
    - [Template Name Lookup](#template-name-lookup)
    - [Associated Constants](#associated-constants)
    - [Generic Entities](#generic-entities)
        - [Generic Classes](#generic-classes)
        - [Generic Choice Types](#generic-choice-types)
        - [Generic Interfaces](#generic-interfaces)
        - [Generic Implementations](#generic-implementations)
    - [Other Features](#other-features)
    - [Generic Type Equality and `observe` Declarations](#generic-type-equality-and-observe-declarations)
    - [Operator Overloading](#operator-overloading)
        - [Common Type](#common-type)
- [Bidirectional Interoperability with C and C++](#bidirectional-interoperability-with-c-and-c)
    - [Goals](#goals)
    - [Non-Goals](#non-goals)
    - [Importing and `#include`](#importing-and-include)
    - [ABI and Dynamic Linking](#abi-and-dynamic-linking)
    - [Operator Overloading](#operator-overloading-1)
    - [Templates](#templates)
    - [Standard Types](#standard-types)
    - [Inheritance](#inheritance-1)
    - [Enums](#enums)
- [Memory Management](#memory-management)
    - [Manual Memory Management](#manual-memory-management)
    - [Automatic Memory Management](#automatic-memory-management)
    - [Safe Memory Management](#safe-memory-management)
    - [Unsafe Memory Management](#unsafe-memory-management)
    - [Generational Reference Counting](#generational-reference-counting)
    - [Regions and Reference Handling](#regions-and-reference-handling)
    - [Memory Safety Guarantees](#memory-safety-guarantees)
- [Error Handling](#error-handling)
    - [Exception Handling](#exception-handling)
    - [`Result` Type](#result-type)
    - [`Option` Type](#option-type)
    - [`panic`](#panic)
- [Concurrency](#concurrency)
    - [Threads](#threads)
    - [Async/Await](#asyncawait)
    - [Synchronization Primitives](#synchronization-primitives)
    - [Parallel Iteration](#parallel-iteration)
    - [Futures](#futures)
- [Standard Library](#standard-library)
    - [Modules and Functions](#modules-and-functions)
    - [Collections](#collections)
    - [I/O](#io)
    - [Networking](#networking)
    - [Utilities](#utilities)
    - [Error Handling](#error-handling-1)
    - [Concurrency Utilities](#concurrency-utilities)
- [Extensions](#extensions)
    - [Language Extensions](#language-extensions)
    - [Library Extensions](#library-extensions)
    - [Tooling Extensions](#tooling-extensions)
- [Tools and Ecosystem](#tools-and-ecosystem)
    - [Compiler](#compiler)
    - [Build System](#build-system)
    - [Debugging Tools](#debugging-tools)
    - [Documentation Tools](#documentation-tools)
    - [Package Manager](#package-manager)
    - [Testing Framework](#testing-framework)
    - [Language Server](#language-server)
- [Getting Started](#getting-started)
    - [Setting Up](#setting-up)
    - [Writing Your First Program](#writing-your-first-program)
    - [Tutorials and Examples](#tutorials-and-examples)
    - [Community and Support](#community-and-support)
- [Best Practices](#best-practices)
    - [Coding Conventions](#coding-conventions)
    - [Performance Optimization](#performance-optimization)
    - [Security Considerations](#security-considerations)
    - [Documentation Standards](#documentation-standards)


## Introduction

This document outlines the design of the Luminar programming language, focusing on its principles and features. It provides an overview intended for both developers working on Luminar and those interested in its design.

This document is _not_ a complete programming manual and does not cover all details or justifications for design decisions. For comprehensive information, refer to the dedicated design documents linked throughout this file.

### This Document is Provisional

The content here includes provisional or placeholder material. Features, syntax, and other aspects may change as the Luminar language evolves. Provisional content is marked as such based on current design discussions.

### Tour of the Basics

Here's a basic example of Luminar code:

```luminar
import Math;

// Returns the smallest factor of `n` > 1,
// and whether `n` itself is prime.
fn SmallestFactor(n: i32) -> (i32, bool) {
  let limit: i32 = Math.Sqrt(n) as i32;
  var i: i32 = 2;
  while (i <= limit) {
    let remainder: i32 = n % i;
    if (remainder == 0) {
      Luminar.Print("{0} is a factor of {1}", i, n);
      return (i, false);
    }
    if (i == 2) {
      i = 3;
    } else {
      // Skip even numbers once we get past `2`.
      i += 2;
    }
 

 }
  return (n, true);
}
```

### Key Features

- **Memory Management**: Luminar supports a hybrid approach with manual, automatic, and safe memory management, including generational reference counting and explicit reference handling.
- **Generics**: Luminar allows defining generic types and functions with constraints, supporting interfaces, and implementations.
- **Concurrency**: Features include threads, async/await, synchronization primitives, and parallel iteration.
- **Error Handling**: Includes exception handling, `Result`, and `Option` types.
- **Standard Library**: Extensive standard library with collections, I/O, networking, and concurrency utilities.

## Code and Comments

Luminar supports single-line and multi-line comments:

```luminar
// This is a single-line comment.

/* This is a
   multi-line comment. */
```

## Build Modes

Luminar provides several build modes for compiling programs:

- **Debug Mode**: Includes debugging information, no optimizations.
- **Release Mode**: Optimizes for performance, omits debugging information.
- **Test Mode**: Similar to debug mode but includes additional test instrumentation.

Switch between build modes using the `luminarc` command-line tool.

## Types are Values

In Luminar, types themselves are first-class values. This means they can be passed to functions, stored in variables, and used in expressions.

### Values Usable as Types

The following kinds of values can be used as types:

- **Primitive Types**: Such as `i32`, `f64`, `bool`, etc.
- **Composite Types**: Tuples, structs, arrays, etc.
- **User-Defined Types**: Classes, enums, choice types, etc.
- **Generics**: Parameterized types and functions.

## Primitive Types

### `bool`

The `bool` type represents Boolean values:

```luminar
let isTrue: bool = true;
let isFalse: bool = false;
```

### Integer Types

Luminar includes several integer types, both signed and unsigned:

- Signed: `i8`, `i16`, `i32`, `i64`
- Unsigned: `u8`, `u16`, `u32`, `u64`

#### Integer Literals

Integer literals can be written in decimal, hexadecimal, octal, or binary notation:

```luminar
let dec: i32 = 42;
let hex: i32 = 0x2A;
let oct: i32 = 0o52;
let bin: i32 = 0b101010;
```

### Fixed-Point Types

Fixed-point numbers provide a middle ground between integer and floating-point arithmetic. Luminar offers:

- `f32` and `f64`: Floating-point types.
- `fx16`, `fx32`, etc.: Fixed-point types with customizable precision.

#### Fixed-Point Literals

Fixed-point literals are written similarly to floating-point literals but with an `fx` suffix:

```luminar
let pi: fx32 = 3.14159fx;
```

### String Types

Luminar includes both mutable and immutable string types:

- `string`: Immutable UTF-8 encoded strings.
- `mut string`: Mutable strings.

#### String Literals

String literals can be enclosed in double quotes:

```luminar
let greeting: string = "Hello, World!";
```

Multi-line strings are supported with triple quotes:

```luminar
let poem: string = """
Roses are red,
Violets are blue,
Luminar is neat,
And so are you.
""";
```

## Values, Objects, and Expressions

### Expression Categories

Luminar expressions are categorized into:

- **Prvalue**: Pure values, such as literals and results of expressions.
- **Xvalue**: "Expiring" values, often the result of move operations.
- **Lvalue**: References to objects with storage duration.

### Expression Phases

Expressions in Luminar go through several phases:

- **Evaluation**: Produces a value or object.
- **Conversion**: Converts values between types.
- **Initialization**: Assigns a value to a variable or object.

## Composite Types

### Tuples

Tuples group multiple values into a single composite value:

```luminar
let point: (i32, i32) = (10, 20);
```

### Struct Types

Structs are user-defined composite types:

```luminar
struct Point {
    x: i32;
    y: i32;
}
```

### Pointer Types

Pointers in Luminar are strongly typed and must be explicitly dereferenced:

```luminar
let p: *i32 = &x;
```

### Arrays and Slices

Arrays have a fixed size, while slices are dynamically sized:

```luminar
let arr: [i32; 4] = [1, 2, 3, 4];
let slice: []i32 = arr[..2];
```

## Expressions

Expressions form the core of Luminar programs, representing computations that evaluate to values. Expressions include operations, function calls, and more.

## Declarations, Definitions, and Scopes

### Declarations

Declarations introduce names and specify their types:

```luminar
let x: i32;
```

### Definitions

Definitions provide the value for a declared name:

```luminar
let x: i32 = 10;
```

### Scopes

Luminar uses lexical scoping, meaning that the scope of a name is determined by the program's textual structure.

## Patterns

### Binding Patterns

Binding patterns are used to introduce new variables:

```luminar
let (a, b) = (1, 2);
```

### Destructuring Patterns

Destructuring patterns break down composite values:

```luminar
let (x, y) = point;
```

### Refutable Patterns

Refutable patterns may fail to match, often used with `match` statements.

## Name-Binding Declarations

### Constant `let` Declarations

`let` declarations introduce immutable bindings:

```luminar
let x = 10;
```

### Variable `var` Declarations

`var` declarations introduce mutable bindings:

```luminar
var y = 20;
```

### `auto`

`auto` is used for type inference:

```luminar
auto z = x + y;
```

### Global Constants and Variables

Global constants and variables are declared at the top level of a package:

```luminar
const MAX_SIZE: i32 = 1024;
var counter: i32 = 0;
```

## Functions

### Parameters

Function parameters are passed by value unless explicitly declared as references:

```luminar
fn Add(x: i32, y: i32) -> i32 {
    return x + y;
}
```

### `auto` Return Type

The `auto` keyword can infer the return type:

```luminar
fn Sum(a: i32, b: i32) -> auto {
    return a + b;
}
```

### Blocks and Statements

Blocks group multiple statements and expressions:

```luminar
fn DoSomething() {
    let x = 10;
    let y = 20;
    // do something with x and y
}
```

### Control Flow

#### `if` and `else`

```luminar
if (x > y) {
    // true branch
} else {
    // false branch
}
```

#### Loops

##### `while`

```luminar
while (x < 10) {
    x += 1;
}
```

##### `for`

```luminar
for (i in 0..10) {
    Luminar.Print(i);
}
```

##### `break`

```luminar
break; // exits the loop
```

##### `continue`

```luminar
continue; // skips to the next iteration
```

#### `return`

```luminar
return x + y;
```

##### `returned var`

Returning a reference to a mutable variable is done with `returned var`:

```luminar
returned var x;
```

#### `match`

`match` is similar to `switch` in other languages:

```luminar
match x {
    1 => Luminar.Print("One"),
    2 => Luminar.Print("Two"),
    _ => Luminar.Print("Other")
}
```

## User-Defined Types

### Classes

Classes define complex types:

```luminar
class Point {
    var x: i32;
    var y: i32;
    
    fn New(x: i32, y: i32) -> Point {
        return Point { x, y };
    }
    
    fn Move(self, dx: i32, dy: i32) {
        self.x += dx;
        self.y += dy;
    }
}
```

#### Assignment

Assignment is done using `=`:

```luminar
point = Point.New(0, 0);
```

#### Class Functions and Factory Functions

Class functions operate on class types:

```luminar
fn Point.New(x: i32, y: i32) -> Point {
    return Point { x, y };
}
```

#### Methods

Methods are functions defined within a class:

```luminar
fn Point.Move(self, dx: i32, dy: i32) {
    self.x += dx;
    self.y += dy;
}
```

#### Inheritance

Inheritance allows a class to extend another:

```luminar
class ColoredPoint : Point {
    var color: string;
}
```

#### Access Control

Access control keywords like `public`, `private`, and `protected` determine the visibility of class members:

```luminar
class Point {
    private var x: i32;
    private var y: i32;
}
```

#### Destructors

Destructors clean up resources:

```luminar
fn Point.Destroy(self)

 {
    // cleanup code
}
```

### Generics

Generics allow types and functions to be parameterized:

```luminar
fn Swap<T>(a: T, b: T) {
    let temp = a;
    a = b;
    b = temp;
}
```

### Type Constraints

Type constraints restrict the types that can be used with generics:

```luminar
fn Add<T: Addable>(x: T, y: T) -> T {
    return x + y;
}
```

### Enums

Enums represent a collection of named values:

```luminar
enum Color {
    Red,
    Green,
    Blue
}
```

#### Enum Matching

Enums are matched using `match`:

```luminar
match color {
    Red => Luminar.Print("Red"),
    Green => Luminar.Print("Green"),
    Blue => Luminar.Print("Blue")
}
```

## Traits and Interfaces

### Defining Traits

Traits define shared behavior for types:

```luminar
trait Printable {
    fn Print(self);
}
```

### Implementing Traits

Implement traits for specific types:

```luminar
impl Printable for Point {
    fn Print(self) {
        Luminar.Print("Point(", self.x, ", ", self.y, ")");
    }
}
```

### Type Classes

Type classes group types with shared functionality:

```luminar
class Addable {
    fn Add(self, other: Addable) -> Addable;
}
```

### Interfaces

Interfaces define behavior for types without implementation:

```luminar
interface Drawable {
    fn Draw(self);
}
```

### Implementing Interfaces

Implement interfaces for types:

```luminar
impl Drawable for Circle {
    fn Draw(self) {
        // drawing code
    }
}
```

### Abstract Classes

Abstract classes cannot be instantiated directly and must be subclassed:

```luminar
abstract class Shape {
    fn Area(self) -> f64;
}
```

### Implementing Abstract Classes

Subclasses implement abstract methods:

```luminar
class Circle : Shape {
    fn Area(self) -> f64 {
        return PI * self.radius * self.radius;
    }
}
```

## Modules and Packages

### Modules

Modules organize code into logical units:

```luminar
module geometry {
    class Point { ... }
    class Circle { ... }
}
```

### Importing Modules

Import modules to use their contents:

```luminar
import geometry::Point;
```

### Packages

Packages group related modules and libraries.

## Error Handling

### `try` and `catch`

Handle errors with `try` and `catch`:

```luminar
try {
    // code that may throw
} catch (e) {
    // handle error
}
```

### `Result` and `Option`

Use `Result` for functions that may fail:

```luminar
fn Divide(x: i32, y: i32) -> Result<i32, string> {
    if (y == 0) {
        return Err("division by zero");
    }
    return Ok(x / y);
}
```

Use `Option` for values that may be absent:

```luminar
fn Find(name: string) -> Option<PhoneNumber> {
    // search logic
    return Some(number);
}
```

## Concurrency and Parallelism

### Threads

Create and manage threads:

```luminar
let handle = spawn {
    // thread code
};
handle.Join();
```

### Async and Await

Asynchronous code is supported with `async` and `await`:

```luminar
async fn FetchData(url: string) -> Result<Data, Error> {
    // async code
}
```

### Channels

Channels facilitate communication between threads:

```luminar
let (tx, rx) = channel();
tx.Send("Hello");
let msg = rx.Receive();
```

### Mutexes and Locks

Synchronize access to shared resources:

```luminar
let lock = Mutex::New();
let data = lock.Lock();
```

### Atomic Types

Atomic types support lock-free concurrent programming:

```luminar
let counter = AtomicI32::New(0);
counter.Increment();
```

## Inline Assembly

Use inline assembly for low-level programming:

```luminar
asm {
    mov eax, 1;
    xor ebx, ebx;
    int 0x80;
}
```

## Testing

### Unit Tests

Unit tests verify individual functions:

```luminar
fn TestAdd() {
    assert(Add(1, 2) == 3);
}
```

### Integration Tests

Integration tests verify the interaction of multiple components:

```luminar
fn TestApp() {
    // integration test code
}
```

### Property-Based Testing

Generate random test cases based on properties:

```luminar
fn TestCommutativity(a: i32, b: i32) -> bool {
    return Add(a, b) == Add(b, a);
}
```

## Best Practices

### Naming Conventions

- **Types**: `CamelCase`
- **Variables**: `snake_case`
- **Constants**: `SCREAMING_SNAKE_CASE`

### Commenting

Provide clear, concise comments, especially for complex code.

### Code Style

Maintain a consistent code style, following Luminar's official guidelines.

---

This cheat sheet provides a comprehensive overview of Luminar's language features and syntax. Keep it handy while coding for quick reference!