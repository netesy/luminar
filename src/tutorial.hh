//tutorial.hh
#pragma once
#include <string>
#include <vector>

struct TutorialSection {
    std::string title;
    std::string description;
    std::vector<std::string> examples;
};

struct Tutorial {
    std::vector<TutorialSection> sections = {
        {
            "Basic Operations",
            "Learn the fundamental operations in Luminar",
            {
                "10 + 5 * 2;       // Basic arithmetic",
                "\"Hello, \" + \"Luminar!\"  // String concatenation",
                "var x: int = 42;  // Variable declaration"
            }
        },
        {
            "Data Types",
            "Understanding primitive and user-defined types in Luminar",
            {
                "type int = i64;   // Default integer type",
                "type str = string; // String type",
                "type Result<T, E> = Success(T) | Error(E);  // Generic type"
            }
        },
        {
            "Control Structures",
            "Understanding loops and conditional statements",
            {
                "for (i in 1..5) { print(i); }",
                "if x > 0 { print(\"Positive\"); }",
                "while count < 3 { print(count); count += 1; }"
            }
        },
        {
            "Functions",
            "Creating and using functions",
            {
                "fn factorial(n: int): int { if n <= 1 { return 1; } return n * factorial(n - 1); }",
                "fn greet(name: str) { print(\"Hello, {name}!\"); }"
            }
        },
        {
            "Object-Oriented Programming",
            "Working with classes and objects",
            {
                "class Vehicle { var name: str; Vehicle(name: str) { self.name = name; } }",
                "var car = Vehicle(\"Toyota\");"
            }
        },
        {
            "Modules",
            "Organizing code using modules",
            {
                "module MathUtils { fn add(a: int, b: int): int { return a + b; } }",
                "import MathUtils; print(MathUtils.add(5, 3));"
            }
        },
        {
            "Error Handling",
            "Handling errors using option types and pattern matching",
            {
                "fn divide(a: int, b: int): int? { if b == 0 { return error(\"Division by zero\"); } return a / b; }",
                "match divide(10, 0) { Some(result) => print(\"Result: {result}\"), None => print(\"Error: Division by zero\") }"
            }
        },
        {
            "Concurrency",
            "Running multiple tasks concurrently",
            {
                "var shared_counter: Atomic<int> = Atomic(0);",
                "fn increment_counter() { for (_ in 1..1000) { shared_counter.fetch_add(1); } }",
                "concurrent { increment_counter(), increment_counter() }",
                "print(\"Final counter value: {shared_counter.get()}\");"
            }
        },
        {
            "Parallel Processing",
            "Executing tasks in parallel",
            {
                "parallel(tasks) { for (var i = 1; i <= 3; i++) { spawn_task(i); } }",
                "fn parallel_task(id: int): None { print(\"Executing task {id}\"); sleep(randint(1, 3)); print(\"Task {id} completed\"); }"
            }
        },
        {
            "Pattern Matching",
            "Using pattern matching for control flow",
            {
                "fn match_example(value: Any): void {",
                "  match value {",
                "    int => print(\"Integer: {value}\"),",
                "    str => print(\"String: {value}\"),",
                "    list<int> => print(\"List of integers: {value}\"),",
                "    dict<str, int> => print(\"Dictionary: {value}\"),",
                "    _ => print(\"Unknown type\")",
                "  }",
                "}"
            }
        }
    };
};
