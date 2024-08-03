# Luminar Programming Language

Luminar is a statically typed programming language designed for readability, efficiency, and modern features. It is intended to be interpreted for development and compiled for release. This README provides a structured overview to help you learn Luminar effectively.

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
  var mut total: int = 0;
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
fn greet(name: *string) -> nil {
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
fn parallel_task(id: int) {
    print("Executing parallel task {id}");


    // Simulate some computation or I/O operation
    sleep(randint(1, 3));  // Sleep for a random duration between 1 and 3 seconds
    print("Parallel task {id} completed");
}

// Execute multiple tasks in parallel
parallel(tasks, cores=Auto, on_error=Auto) {
    for (var i = 1; i <= 5; i++) {
        run_task(i);
    }
}
```

## Memory Management

### **Introduction**

Luminar provides two memory management modes:
1. **Safe Mode:** Uses automatic memory management with implicit handling of `new`, `move`, and `borrow`.
2. **Unsafe Mode:** Provides fine-grained control over memory with explicit allocation and deallocation using raw pointers.

---

### **1. Safe Mode (Normal Mode)**

In Safe Mode, memory management is handled automatically. Luminar’s compiler takes care of allocation, reference counting, and deallocation behind the scenes. The syntax is simplified to avoid manual memory management tasks.

#### **Safe Mode: Image Struct Definition**

```luminar
struct Image {
    data: Array<u8>,
    width: u32,
    height: u32,
}

impl Image {
    // Constructor to create a new Image
    fn new(width: u32, height: u32) -> Image {
        var data = Array::new(width * height); // Allocate memory for image data
        Image {
            data: data,
            width: width,
            height: height,
        }
    }

    // Method to set a pixel value
    fn set_pixel(&mut self, x: u32, y: u32, value: u8) {
        let index = y * self.width + x;
        self.data[index] = value;
    }

    // Method to get a pixel value
    fn get_pixel(&self, x: u32, y: u32) -> u8 {
        let index = y * self.width + x;
        self.data[index]
    }
}
```

**Explanation:**

- **`Array::new(width * height)`**: Allocates memory for image data. The memory is managed automatically.
- **`fn new(width: u32, height: u32) -> Image`**: Creates a new `Image` instance with allocated memory.
- **`fn set_pixel` and `fn get_pixel`**: Methods for modifying and accessing pixel values. No need to manually manage memory.

#### **Safe Mode: Usage Example**

```luminar
fn main() {
    var img = Image::new(100, 100);  // Create a new image of 100x100 pixels

    img.set_pixel(10, 10, 255);  // Set pixel at (10, 10) to value 255
    print(img.get_pixel(10, 10));  // Output: 255

    // No explicit deallocation needed; memory is managed automatically
}
```

**Explanation:**

- **`var img = Image::new(100, 100)`**: Creates an `Image` with automatic memory management.
- **`img.set_pixel` and `print(img.get_pixel)`**: Methods to interact with the image.

---

### **2. Unsafe Mode (Manual Allocation)**

In Unsafe Mode, you have explicit control over memory. You are responsible for allocating and deallocating memory using raw pointers. This mode is useful for performance-critical sections where fine-grained control is needed.

#### **Unsafe Mode: Image Struct Definition**

```luminar
struct Image {
    data: *u8,    // Raw pointer for image data
    width: u32,
    height: u32,
}

impl Image {
    // Constructor to create a new Image
    fn new(width: u32, height: u32) -> Image {
        unsafe {
            var size = width * height;
            var data = new u8 * size;  // Allocate memory for image data
            if data == 0 {
                // Handle memory allocation failure
                panic("Failed to allocate memory for image data");
            }
            }
            Image {
                data: data,
                width: width,
                height: height,
            }
        }
    }

    // Method to set a pixel value
    fn set_pixel(&mut self, x: u32, y: u32, value: u8) {
        unsafe {
            let index = y * self.width + x;
            *(self.data + index) = value;
        }
    }

    // Method to get a pixel value
    fn get_pixel(&self, x: u32, y: u32) -> u8 {
        unsafe {
            let index = y * self.width + x;
            *(self.data + index)
        }
    }
    
// Destructor to deallocate memory
fn free(&mut self) {
    unsafe {
        if self.data != 0 {
            deallocate(self.data);  // Manually free the allocated memory
            self.data = 0;         // Set pointer to zero after deallocation
        }
    }
}
}
```

**Explanation:**

- **`unsafe { var data = new u8 * size; }`**: Manually allocates memory for image data.
- **`fn set_pixel` and `fn get_pixel`**: Use raw pointers and pointer arithmetic to access and modify memory.

#### **Unsafe Mode: Usage Example**

```luminar
fn main() {
    unsafe {
        var img = Image::new(100, 100);  // Create a new image of 100x100 pixels

        img.set_pixel(10, 10, 255);  // Set pixel at (10, 10) to value 255
        print(img.get_pixel(10, 10));  // Output: 255

        img.free();  // Manually free the allocated memory
    }
}
```

**Explanation:**

- **`unsafe { var img = Image::new(100, 100); }`**: Allocates memory manually.
- **`img.free();`**: Frees the allocated memory, which is done manually in Unsafe Mode.

---

### **Comparing Safe and Unsafe Modes**

**Safe Mode:**
- **Advantages:** 
  - Simplifies memory management.
  - Reduces risk of memory leaks and undefined behavior.
  - Automatic memory deallocation.
- **When to Use:** 
  - For most general-purpose programming where safety and simplicity are important.
  - When you prefer automatic management and safety over manual control.

**Unsafe Mode:**
- **Advantages:**
  - Provides fine-grained control over memory allocation and deallocation.
  - Potential for performance optimization.
- **When to Use:**
  - For performance-critical sections where manual control is needed.
  - When dealing with low-level operations or interfacing with hardware.
- **Best Practices::**
  - Check for allocation failures.
  - Avoid dangling pointers.
  - Minimize unsafe code.

---

### **Conclusion**

Luminar’s approach to memory management offers flexibility through its safe and unsafe modes. Safe Mode provides automatic memory management with simplified syntax, while Unsafe Mode allows explicit control for performance optimizations. Understanding when and how to use each mode can help you effectively manage resources in your Luminar applications.

## Example Code Snippets

Refer to the provided code examples throughout the document for demonstrations of various functionalities.

## Contributing

Luminar is an open-source project. Contributions to its development are welcome. Stay tuned for updates and additional documentation as the language evolves.
