Here is the updated README for the Luminar programming language, incorporating the new memory management system:

---

# Luminar Programming Language

Luminar is a statically typed programming language designed for readability, efficiency, and modern features. It is intended to be interpreted for development and compiled for release. This README provides a structured overview to help you learn Luminar effectively, with a focus on its advanced memory management system.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Syntax Overview](#syntax-overview)
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
13. [Example Code Snippets](#example-code-snippets)
14. [Contributing](#contributing)

## Getting Started

Luminar is under development. Installation instructions will be provided once the language becomes publicly available.

## Syntax Overview

### Comments

Comments can be added using:
```luminar
// This is a single-line comment
/* This is a multi-line comment */
/** This is a documentation comment */
```

### Basic Syntax

#### Arithmetic Operations

```luminar
print(1 + 2 + (3 * 4) - (10 / 5));  // Output: 11
```

#### Logical Operators

```luminar
print((12 > 3) and (12 > 13));    // false (AND requires both conditions true)
print((12 > 3) or (12 > 13));     // true (OR requires at least one condition true)
print(!true);                     // false
print(!false);                    // true
```

#### Unary Plus and Minus

```luminar
print(+10);  // 10 (unary plus is a no-op)
print(-5);   // -5
```

#### String Concatenation

```luminar
print("Hello" + " " + "World");  // Output: Hello World
```

## Data Types

### Basic Data Types

- **Nil**: Represents the absence of a value.
- **Bool**: Boolean type.
- **Integers**:
  - `int`: Platform-dependent signed integer
  - `uint`: Platform-dependent unsigned integer
  - `i8, u8, i16, u16, i32, u32, i64, u64`
- **Floating-Point Numbers**:
  - `f32, f64`
  - `float`: Platform-dependent floating point
- **String**: Represents a sequence of characters.
  - `str`

### Composite Types

- **List**: A list of elements of a specific type.
- **Dict**: A dictionary with keys and values of specific types.
- **Enum**: An enumeration of predefined values.
- **Function**: A function type with parameter types and a return type.
- **Sum**: A sum type (tagged union or variant) that can hold any one of several specified types.
- **Union**: A union type that represents a value that can be one of several types.
- **UserDefined**: A user-defined type allowing custom structured types.
- **Optional**: Represented by `?`, allowing for optional values.

## Operators

- **Arithmetic Operators**: `+, -, *, /, %`
- **Comparison Operators**: `==, !=, <, >, <=, >=`
- **Logical Operators**: `&& (and), || (or), ! (not)`
- **String Concatenation**: `+`

## Control Flow Structures

### For Loop Over a Range

```luminar
for (i in 1..10) {
  print("Iteration: {i}");
}
```

### For Loop Over a List

```luminar
var my_list: list<int> = (1, 2, 3, 4, 5);
for (key in my_list) {
  print("{key}");
}
```

### Iterate Over a Dictionary

```luminar
var my_dict: dict<str, int> = {
  "a": 1,
  "b": 2,
  "c": 3,
};

for ((key, value) in my_dict) {
  print("{key}: {value}");
}
```

### While Loop

```luminar
var j: int = 0;
while (j < 5) {
  print("While loop iteration: {j}");
  j += 1;
}
```

## Error Handling

### Error Handling in Functions

```luminar
fn factorial(n: int): int? {
  if (n < 0) {
    return error("negative input"); // Use specific error return type
  } elif (n == 0) {
    return 1;
  } else {
    return n * factorial(n - 1);
  }
}

// Function to sum elements of a list with dependent type checking
fn sum(list: list<int?>): int {
  var total: int = 0;
  for (item in list) {
    match (item) {
      Some(value) => total += value,  // Handle Some cases (values present)
      None => continue,               // Skip None values (missing values)
    }
  }
  return total;
}
```

## Functions

### Function with Optional Parameter

```luminar
fn greeting(name: str, date: str? = None): str {
  const var name: str = "John"; 
  var message: str = format!("Hello {name}");

  if date.is_some() {
    var date_str: str = date.unwrap();
    message = format!("{message}, today's date is {date_str.to_date()}");
  }

  return message;
}

// Print greetings with and without date:
print(greeting(name = "John"));
print(greeting(name = "John", date = "12/05/2024"));
```

### Basic Functions

```luminar
// Function returning an integer
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

// Function taking a string reference (pointer) and returning nil
fn greet(ref name: string) -> nil {
    print("Hello, " + *name);
}
```

### Error Handling in Functions

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
    print("Result: " + result);
} else {
    print("Error: division by zero");
}
```

## Classes

### Defining a Class

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

// Object of class Person:
var person1: Person = Person("Alice", 30);
person1.display();  // Output: Name: Alice, Age: 30
```

## Modules

### Organizing Code into Modules

```luminar
module Sample {
  // Module contents here
}
```

## Pattern Matching

### Example of Pattern Matching

```luminar
fn match_example(value: Any): void { // Use Any for untyped values (caution advised)
  match value {
    int => print("The value is an integer"),
    str => print("The value is a string"),
    list<T> => print("The value is a list"), // Placeholder for generic type T
    dict<K, V> => print("The value is a dictionary"), // Placeholders for generic types K and V
    _ => print("Unknown type"),
  }
}

// Usage example:
match_example(42);
match_example("Hello");
match_example(vec!(1, 2, 3));
match_example(map!("a" => 1));
match_example(true);
```

## Concurrency and Parallelism

### Concurrency Example

Luminar provides planned features for concurrency, allowing for simultaneous execution of tasks. Here's a simple example of defining a function for concurrent execution:

```luminar
// Define a function for concurrent execution
fn concurrent_task(id: int): None {
    print("Executing concurrent task {id}");
    // Simulate some computation or I/O operation
    sleep(randint(1, 3));  // Sleep for a random duration between 1 and 3 seconds
    print("Concurrent task {id} completed");
}

// Execute multiple tasks concurrently
concurrent(tasks, cores=Auto, on_error=Auto) {
    for (var i = 1; i <= 3; i++) {
        run_task(i);
    }
}
```

### Parallelism Example

Luminar also supports parallel execution of tasks. Here's an example of defining a function for parallel execution:



```luminar
// Define a function for parallel execution
fn parallel_task(id: int): None {
    print("Executing parallel task {id}");
    // Simulate some computation or I/O operation
    sleep(randint(1, 3));  // Sleep for a random duration between 1 and 3 seconds
    print("Parallel task {id} completed");
}

// Execute multiple tasks in parallel
parallel(tasks) {
    for (var i = 1; i <= 3; i++) {
        spawn_task(i);
    }
}
```

## Memory Management

Luminar's memory management is designed to be robust and efficient, combining linear-first and reference-first approaches along with manual management in unsafe code.

### Linear-First Approach

- **Regions**: Memory is managed within defined regions, ensuring automatic deallocation when regions go out of scope. This provides predictable and safe memory usage.
- **Automatic Management**: Regions and variables are automatically managed, reducing the risk of memory leaks and dangling pointers.

### Reference-First Approach

- **References (`ref`)**: All references are mutable by default, simplifying reference handling.
- **Linear Conversion (`lin`)**: Linear conversion from references to linear types is explicit using the `lin` keyword, allowing optimization of critical sections.

### Unsafe Mode

- **Manual Management**: In `unsafe` blocks, memory management can be performed manually using raw pointers (`*u8`). This is suitable for performance-critical tasks and low-level operations.

#### Example of Unsafe Mode

```luminar
unsafe {
  var buffer: *u8 = allocate(1024);  // Allocate 1024 bytes of memory
  // Perform manual memory operations
  deallocate(buffer);  // Deallocate memory when done
}
```

## Example Code Snippets

### Example 1: Basic Memory Management

```luminar
var x: int = 10;
{
  var y: int = 20;
  print(x + y);  // Output: 30
}  // y goes out of scope here
```

### Example 2: Using `ref` and `lin`

```luminar
fn manipulate_data(ref data: list<int>) -> int {
  // Convert reference to linear type for modification
  var linear_data: lin list<int> = data;
  linear_data.push(42);
  return linear_data.sum();
}

var my_list: list<int> = (1, 2, 3);
print(manipulate_data(ref my_list));  // Output: 6 (1+2+3+42)
```

## Contributing

Contributions are welcome! Please submit issues, feature requests, or pull requests to the project's repository. For more detailed contribution guidelines, refer to the CONTRIBUTING.md file in the repository.

---

Feel free to adapt the README based on additional details or changes specific to your Luminar project.