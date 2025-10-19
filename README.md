# Multithreading

<details>

<summary>1. Thread implementation [demo_001.cpp]</summary>

## Overview

This program demonstrates basic multithreading in C++ using the `<thread>` library. It creates two concurrent threads that print messages to the console: the main thread and a spawned worker thread.

## What This Code Does

The program spawns a secondary thread (`t1`) that runs the `func1()` function while the main thread continues its own execution. Both threads attempt to print 100 messages to the console:

- **Thread T1**: Prints `--T1` 100 times
- **Main Thread**: Prints `MAIN--` 100 times

Because both threads write to `std::cout` concurrently, the output will be interleaved in an unpredictable pattern.

## Key Concepts Demonstrated

### 1. **Thread Creation**
```cpp
std::thread t1(func1);
```
Creates a new thread that immediately begins executing the `func1()` function.

### 2. **Thread Joining**
```cpp
t1.join();
```
The main thread waits for thread `t1` to complete before continuing. This is crucial because:
- A thread object must be either joined or detached before it's destroyed
- Failing to join/detach causes the program to call `std::terminate()`

### 3. **Exception-Safe Thread Management**
The try-catch block ensures that even if an exception occurs in the main thread:
- The spawned thread `t1` is properly joined (cleanup)
- The exception is then re-thrown for proper error handling
- This prevents thread leaks and undefined behavior

## Building and Running

### Compilation
```bash
g++ -std=c++11 -pthread demo_001.cpp -o threading_demo
```

### Execution
```bash
./threading_demo
```

## Important Notes

- **Thread Safety**: This code has a race condition. Multiple threads writing to `std::cout` simultaneously can cause garbled output. For production code, use proper synchronization (mutexes) when accessing shared resources.
- **C++11 Required**: The `<thread>` library requires C++11 or later.
- **Compilation Flag**: Use `-pthread` flag when compiling with GCC/Clang to link the threading library.

## Learning Points

This example is ideal for understanding:
- How to create and manage threads in C++
- Why proper thread joining is essential
- The importance of exception-safe thread management
- The concept of concurrent execution and race conditions

</details>

<details>

<summary>2. Sending Functors into Threads [demo_002.cpp]</summary>

## Overview

This program demonstrates advanced C++ threading concepts including functors, parameter passing methods, move semantics, and thread management. It's structured as six progressive demonstrations that build upon each other to teach different aspects of multithreading.

## What This Code Demonstrates

### Demo 1: Basic Functor Threading
Shows how to use a functor (function object) with threads. A functor is created, then passed to a thread constructor.

```cpp
MyFunctor1 fuctr;
std::thread t(fuctr);
```

### Demo 2: Inline Functor Creation
Demonstrates creating a temporary functor object directly in the thread constructor. The extra parentheses `((MyFunctor1()))` prevent the "most vexing parse" - a C++ ambiguity where the compiler might interpret it as a function declaration.

```cpp
std::thread t((MyFunctor1()));
```

### Demo 3: Passing Arguments by Reference
Uses `std::ref()` to pass a reference to a thread. Both the main thread and the spawned thread can access and modify the same variable.

```cpp
std::string mytext = "message to T";
std::thread t((MyFunctor2()), std::ref(mytext));
```

**Key Point**: The variable remains valid and accessible in the main thread.

### Demo 4: Move Semantics
Demonstrates `std::move()` to transfer ownership of a variable to the thread. After moving, the original variable becomes empty (moved-from state).

```cpp
std::thread t((MyFunctor3()), std::move(mytext));
// mytext is now empty!
```

**Important**: The original comment mentions segmentation fault, but actually, the moved-from string is just empty - printing it is safe but shows nothing.

### Demo 5: Moving Thread Objects
Shows that threads cannot be copied, only moved. This transfers ownership of the thread from one `std::thread` object to another.

```cpp
std::thread t2 = std::move(t);
// t is no longer joinable; must use t2
```

**Key Concept**: `std::thread` is a move-only type. After moving, the original thread object is in a non-joinable state.

### Demo 6: Thread IDs
Demonstrates how to retrieve and track thread identifiers using `get_id()` and `std::this_thread::get_id()`.

```cpp
std::cout << "main thread ID:" << std::this_thread::get_id() << std::endl;
std::cout << "t ID:" << t.get_id() << std::endl;
```

After moving a thread, the original thread object shows an invalid ID, while the new object retains the actual thread ID.

## Key Concepts Explained

### Functors vs Functions
A **functor** is a class that overloads the `operator()`, making objects of that class callable like functions. Functors can maintain state and are ideal for thread operations.

### Most Vexing Parse
The extra parentheses in `std::thread t((MyFunctor1()))` prevent C++ from interpreting this as a function declaration instead of object creation.

### std::ref() vs std::move()
- **`std::ref()`**: Passes a reference wrapper, allowing shared access to the variable
- **`std::move()`**: Transfers ownership, making the original variable invalid

### Thread Move Semantics
Threads cannot be copied (no copy constructor) but can be moved. This ensures only one object manages each thread's lifetime.

### Hardware Concurrency
`std::thread::hardware_concurrency()` returns the number of concurrent threads supported by the hardware (typically the number of CPU cores). This helps avoid **over-subscription** - creating more threads than the system can efficiently handle.

## Building and Running

### Compilation
```bash
g++ -std=c++11 -pthread main.cpp -o threading_demo
```

### Execution
```bash
./threading_demo
```

## Expected Output

**Note**: Thread IDs and their exact output format may vary by system.

## Important Concepts

### Move-Only Types
`std::thread` is move-only to prevent resource management issues:
- Copying would create two objects managing the same thread
- Only one object should be responsible for joining/detaching
- Moving transfers this responsibility safely

### Parameter Passing Safety
When passing parameters to threads:
- **By value**: Safe, but creates a copy
- **By reference with `std::ref()`**: Fast, but requires careful lifetime management
- **By move with `std::move()`**: Efficient for large objects, transfers ownership

### Exception Safety
All demos use try-catch blocks to ensure threads are properly joined even if exceptions occur, preventing resource leaks and undefined behavior.

## Common Pitfalls

1. **Forgetting std::ref()**: Without it, arguments are copied even if the functor expects a reference
2. **Using moved-from objects**: After `std::move()`, the original object is in an unspecified state
3. **Not joining moved threads**: After moving a thread, join the new owner, not the original
4. **Over-subscription**: Creating too many threads can degrade performance

## Learning Objectives

This example teaches:
- How to use functors with threads
- Different methods of passing arguments to threads
- Move semantics in the context of thread management
- Thread ownership and lifetime management
- How to query system threading capabilities
- Exception-safe thread management patterns

## Requirements

- C++11 or later
- POSIX threads library (linked with `-pthread`)
- Compiler: GCC, Clang, or MSVC with C++11 support

</details>