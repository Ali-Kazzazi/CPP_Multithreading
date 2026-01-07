# C++ Multithreading

## Table of Contents

- [1. Thread implementation [demo_001.cpp]](#1-thread-implementation-demo_001cpp)
- [2. Sending Functors into Threads [demo_002.cpp]](#2-sending-functors-into-threads-demo_002cpp)
- [3. Thread Creation Methods [demo_003.cpp]](#3-thread-creation-methods-demo_003cpp)
- [4. Mutex and Thread Synchronization [demo_4.cpp]](#4-mutex-and-thread-synchronization-demo_4cpp)
- [5. Avoiding Data Race: Best Practices [demo_005.cpp]](#5-avoiding-data-race-best-practices-demo_005cpp)
- [6. Deadlock Prevention [demo_006.cpp]](#6-deadlock-prevention-demo_006cpp)
- [7. Shared Mutex (Reader-Writer Lock) [demo_007.cpp]](#7-shared-mutex-reader-writer-lock-demo_007cpp)
---

# 1. Thread implementation [demo_001.cpp]

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




# 2. Sending Functors into Threads [demo_002.cpp]

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
g++ -std=c++11 -pthread demo_002.cpp -o threading_demo
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




# 3. Thread Creation Methods [demo_003.cpp]

## Overview

This program demonstrates the various ways to create threads in C++11 and beyond. It showcases different object passing mechanisms, lambda functions, and member function invocation patterns with threads.

## ⚠️ Critical Bug Warning

**This code has a serious bug**: Threads `t1`, `t2`, `t3`, `t4`, and `t5` are never joined or detached before the program exits. This will cause the program to call `std::terminate()` and crash. In production code, every thread must be either joined or detached before destruction.

## Thread Creation Methods Demonstrated

### 1. Passing Objects by Value (Copy)
```cpp
std::thread t1(cl, 1);
```
- Creates a **copy** of the object `cl`
- Calls `operator()(1)` on the copy
- Safe: the thread has its own independent copy
- Cost: Memory overhead from copying

### 2. Passing Objects by Reference
```cpp
std::thread t2(std::ref(cl), 2);
```
- Wraps `cl` with `std::ref()` to pass by reference
- Calls `operator()(2)` on the original object
- Efficient: No copying overhead
- **Danger**: Must ensure the object outlives the thread

### 3. Passing Objects with Move Semantics
```cpp
std::thread t3(std::move(cl), 3);
```
- Transfers ownership of `cl` to the thread
- Calls `operator()(3)` on the moved object
- After this, `cl` is in a valid but unspecified state
- Efficient for large objects

### 4. Creating Temporary Objects with Arguments
```cpp
std::thread t4(MYCLASS(), 4, "aaaa");
```
- Creates a temporary `MYCLASS` object inline
- Attempts to call `operator()(4, "aaaa")`
- **Note**: This requires a matching operator() signature

### 5. Creating Temporary Objects (Parameterless)
```cpp
std::thread t5(MYCLASS());
```
- Creates a temporary `MYCLASS` object inline
- Calls the parameterless `operator()()`

### 6. Lambda Functions
```cpp
std::thread t6([](std::string s){
    std::cout << "output: " << s+s << std::endl; 
    return s+s;
}, "CO");
```
- Defines an anonymous function inline
- Lambda syntax: `[captures](parameters){ body }`
- Empty `[]` means no captured variables
- Return values from lambdas in threads are ignored
- **This thread is properly joined!**

### 7. Member Functions with Object Copy
```cpp
std::thread t7(&MYCLASS::func1, cl, 7, "CO");
```
- `&MYCLASS::func1` is a pointer to member function
- First argument after the function pointer is the object
- `cl` is **copied** into the thread
- Equivalent to: `(copy_of_cl).func1(7, "CO")`
- **This thread is properly joined!**

### 8. Member Functions with Object Pointer
```cpp
std::thread t8(&MYCLASS::func1, &cl, 8, "CO");
```
- Uses `&cl` (pointer) instead of `cl` (copy)
- Calls member function on the **original** object
- Equivalent to: `(&cl)->func1(8, "CO")`
- More efficient (no copying), but requires careful lifetime management
- **This thread is properly joined!**

## Key Concepts

### Functors (Function Objects)
The `MYCLASS` has overloaded `operator()`, making its objects callable:
```cpp
MYCLASS obj;
obj();       // Calls operator()()
obj(5);      // Calls operator()(int)
```

### Thread Parameters
When you pass arguments to a thread:
1. The first argument is what to execute (function, lambda, functor)
2. Subsequent arguments are passed to that callable
3. Arguments are **copied by default** (use `std::ref()` for references)

### Member Function Pointers
Format: `&ClassName::memberFunction`
```cpp
std::thread t(&MYCLASS::func1, object_or_pointer, args...);
```
- If the second argument is an object: it's copied
- If the second argument is a pointer: the original is used

### Lambda Syntax
```cpp
[capture_list](parameters) -> return_type { body }
```
- `[]` - Capture nothing
- `[=]` - Capture all by value
- `[&]` - Capture all by reference
- `[x, &y]` - Capture x by value, y by reference

## Building and Running

### Compilation
```bash
g++ -std=c++11 -pthread demo_003.cpp -o thread_methods
```

### Execution
```bash
./thread_methods
```

## Expected Output

```
output: COCO
7 CO
8 CO
```

**Then the program will crash** with an error like:
```
terminate called without an active exception
Aborted (core dumped)
```

This happens because threads `t1` through `t5` are never joined.

## Fixed Version

To fix this code, you must join or detach all threads:

```cpp
int main(){
    MYCLASS cl;
    
    std::thread t1(cl, 1);
    std::thread t2(std::ref(cl), 2);
    std::thread t3(std::move(cl), 3);
    std::thread t4(MYCLASS(), 4, "aaaa");  
    std::thread t5(MYCLASS()); 
    
    // Join all threads before they go out of scope
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    
    std::thread t6([](std::string s){
        std::cout << "output: " << s+s << std::endl; 
        return s+s;
    }, "CO");
    t6.join();
    
    std::thread t7(&MYCLASS::func1, cl, 7, "CO");
    t7.join();
    
    std::thread t8(&MYCLASS::func1, &cl, 8, "CO");
    t8.join();
    
    return 0;
}
```

## When to Use Each Method

| Method | Use Case | Performance | Safety |
|--------|----------|-------------|--------|
| Copy (value) | Small objects, thread-local data needed | Slower (copy overhead) | Very safe |
| Reference (`std::ref`) | Large objects, shared state | Fast | Risky (lifetime management) |
| Move (`std::move`) | Unique ownership transfer | Fast | Safe (clear ownership) |
| Pointer | Member functions on existing objects | Fastest | Risky (lifetime management) |
| Lambda | Simple, inline operations | Depends on captures | Flexible |

## Common Pitfalls

1. **Forgetting to join/detach**: Always join or detach threads before they're destroyed
2. **Dangling references**: When using `std::ref()` or pointers, ensure the object outlives the thread
3. **Using moved-from objects**: After `std::move()`, don't use the original object
4. **Lambda captures**: Be careful capturing local variables by reference
5. **Return values**: Thread functions can return values, but they're ignored (use futures for return values)

## Learning Objectives

This example teaches:
- Multiple ways to create threads in C++
- How to pass objects to threads (copy, reference, move)
- How to call member functions in threads
- Lambda functions with threads
- The critical importance of thread lifetime management
- Function objects and operator overloading

## Requirements

- C++11 or later
- POSIX threads library (linked with `-pthread`)
- Compiler: GCC, Clang, or MSVC with C++11 support



# 4. Mutex and Thread Synchronization [demo_4.cpp]

## Overview

This program demonstrates different approaches to thread synchronization in C++, progressing from unsafe code (race conditions) to best practices (RAII and encapsulation). It illustrates why mutexes are necessary and how to use them correctly.

## The Problem: Race Conditions

When multiple threads access shared resources (like `std::cout` or files) simultaneously without synchronization, you get **race conditions** - unpredictable behavior where output gets mixed or corrupted.

## Four Synchronization Approaches

### Demo 1: No Synchronization (❌ Bad)
```cpp
void func1() {
    std::cout << "T1 ---\n";  // Multiple threads writing without protection
}
```

**Problem**: Race condition - output from different threads gets interleaved and garbled.

**Example of bad output**:
```
T1 ----- 
m-ain
T1 --
main--
-
```

### Demo 2: Manual Mutex Locking (⚠️ Acceptable but Risky)
```cpp
void dispMessage1(std::string s) {
    mtx.lock();
    std::cout << s << std::endl;
    mtx.unlock();
}
```

**Advantages**:
- Protects the shared resource
- Prevents race conditions
- Simple to understand

**Problems**:
- If an exception occurs between `lock()` and `unlock()`, the mutex stays locked forever (deadlock)
- Easy to forget to unlock
- Not exception-safe
- Verbose and error-prone

### Demo 3: RAII with lock_guard (✅ Good)
```cpp
void dispMessage2(std::string s) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << s << std::endl;
    // Lock automatically released when 'lock' goes out of scope
}
```

**Advantages**:
- Exception-safe: lock is released even if exceptions occur
- RAII (Resource Acquisition Is Initialization) pattern
- Can't forget to unlock
- Cleaner, more maintainable code

**Use when**: Protecting shared global resources accessed by multiple components

### Demo 4: Encapsulated Mutex (✅ Best Practice)
```cpp
class Logger {
    std::mutex mtx;      // Private mutex
    std::ofstream f;     // Private resource
public:
    void log(std::string s) {
        std::lock_guard<std::mutex> lock(mtx);
        f << s << std::endl;
    }
};
```

**Advantages**:
- Mutex is bound to the resource it protects
- Impossible to access the resource without the mutex
- Encapsulation: users can't misuse the resource
- Each instance has its own mutex (better concurrency)
- Clear ownership and responsibility

**Use when**: You own the resource and want to provide thread-safe access

## Key Concepts

### What is a Mutex?
A **mutex** (mutual exclusion) is a synchronization primitive that ensures only one thread can access a shared resource at a time.

### RAII (Resource Acquisition Is Initialization)
A C++ programming technique where:
1. Resources are acquired in a constructor
2. Resources are released in a destructor
3. Guarantees cleanup even when exceptions occur

`std::lock_guard` is a perfect example of RAII for mutex management.

### Race Condition
When multiple threads access shared data concurrently and at least one modifies it, without proper synchronization. The outcome depends on unpredictable thread scheduling.

### Deadlock
When a mutex is locked but never unlocked, causing all threads waiting for it to hang forever.

## Comparison of Approaches

| Approach | Exception-Safe | Easy to Use | Encapsulation | Recommended |
|----------|----------------|-------------|---------------|-------------|
| No sync | N/A | ✅ | N/A | ❌ Never |
| Manual lock/unlock | ❌ | ⚠️ | ❌ | ❌ Avoid |
| lock_guard | ✅ | ✅ | ⚠️ | ✅ For shared globals |
| Encapsulated | ✅ | ✅ | ✅ | ✅ For owned resources |

## Building and Running

### Compilation
```bash
g++ -std=c++11 -pthread demo_004.cpp -o mutex_demo
```

### Execution
```bash
./mutex_demo
```

The program will:
1. Show garbled output from Demo 1 (race condition)
2. Show clean output from Demo 2 and 3
3. Create `app.log` file with output from Demo 4

### Checking the Log File
```bash
cat app.log
```

## Expected Output

### Demo 1 (Garbled - Race Condition)
```
=== DEMO 1: No Synchronization (Race Condition) ===
T1 ------ 
main-
T1 -
---
-- main
T1 ---
[...mixed output...]
```

### Demo 2 & 3 (Clean - Synchronized)
```
=== DEMO 2: Manual Mutex Locking ===
T2 ---
--- main
T2 ---
--- main
[...clean alternating output...]

=== DEMO 3: RAII Mutex (lock_guard) ===
T2 ---
--- main
T2 ---
[...clean alternating output...]
```

### Demo 4 (app.log file)
```
T1 ---
--- main
T1 ---
--- main
[...all entries cleanly written...]
```

## Best Practices

### 1. Always Use RAII for Mutexes
❌ **Don't do this**:
```cpp
mtx.lock();
do_something();  // If this throws, mutex stays locked!
mtx.unlock();
```

✅ **Do this**:
```cpp
std::lock_guard<std::mutex> lock(mtx);
do_something();  // Exception-safe
```

### 2. Keep Critical Sections Small
```cpp
// ❌ Bad: Locks for too long
std::lock_guard<std::mutex> lock(mtx);
expensive_computation();
std::cout << result << std::endl;

// ✅ Good: Only lock when accessing shared resource
expensive_computation();
{
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << result << std::endl;
}
```

### 3. Bind Mutexes to Resources
```cpp
// ✅ Best: Mutex and resource are encapsulated together
class ThreadSafeQueue {
    std::mutex mtx;
    std::queue<int> q;
public:
    void push(int val) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(val);
    }
};
```

### 4. Avoid Global Mutexes
Global mutexes create unnecessary contention. Use encapsulated mutexes when possible.

### 5. Use std::unique_lock for Advanced Features
If you need to unlock before scope ends or use condition variables:
```cpp
std::unique_lock<std::mutex> lock(mtx);
// Can call lock.unlock() manually if needed
```

## Common Pitfalls

### 1. Forgetting to Unlock (Manual Locking)
```cpp
mtx.lock();
if (error) return;  // BUG: mutex never unlocked!
mtx.unlock();
```
**Solution**: Use `lock_guard` or `unique_lock`

### 2. Locking the Same Mutex Twice (Deadlock)
```cpp
mtx.lock();
function_that_also_locks_mtx();  // Deadlock!
mtx.unlock();
```
**Solution**: Use `std::recursive_mutex` or restructure code

### 3. Accessing Resource Without Mutex
```cpp
class BadLogger {
    std::mutex mtx;
    std::ofstream f;  // public - can be accessed without mutex!
public:
    std::ofstream& getFile() { return f; }  // BAD!
};
```
**Solution**: Keep resources private, only expose thread-safe methods

### 4. Holding Locks Too Long
```cpp
std::lock_guard<std::mutex> lock(mtx);
expensive_network_call();  // Blocks all other threads!
```
**Solution**: Only lock around shared resource access

## When to Use Each Pattern

| Pattern | Use Case |
|---------|----------|
| **No synchronization** | Single-threaded code, thread-local data |
| **Manual lock/unlock** | Never (use RAII instead) |
| **lock_guard** | Simple mutex locking, can't unlock early |
| **unique_lock** | Need to unlock before scope ends, condition variables |
| **Encapsulated mutex** | Own the resource, want to provide thread-safe interface |

## Learning Objectives

This program teaches:
- Why synchronization is necessary (race conditions)
- How mutexes work
- The dangers of manual mutex management
- RAII pattern and its benefits
- Encapsulation as a best practice
- Exception-safe thread synchronization
- Resource ownership and mutex binding

## Requirements

- C++11 or later
- POSIX threads library (linked with `-pthread`)
- Compiler: GCC, Clang, or MSVC with C++11 support

## Further Reading

- `std::unique_lock` for more flexible locking
- `std::condition_variable` for thread signaling
- `std::shared_mutex` for reader-writer locks (C++17)
- `std::scoped_lock` for locking multiple mutexes (C++17)






# 5. Avoiding Data Race: Best Practices [demo_005.cpp]

## Overview

This program demonstrates the three fundamental principles for preventing data races in multithreaded C++ applications. It contrasts unsafe, buggy code with proper, thread-safe implementations to illustrate common pitfalls and their solutions.

## The Three Principles of Data Race Prevention

### 1. Use Mutex to Synchronize Data Access
Every access to shared data must be protected by a mutex. Without synchronization, concurrent reads and writes lead to undefined behavior.

### 2. Never Leak Handles to Data
Never return references or pointers to internal data. Doing so allows external code to bypass mutex protection.

### 3. Design Interface Appropriately
Combine related operations into single atomic actions. Avoid interfaces that require multiple sequential calls where race conditions can occur between calls.

## Code Structure

### Bad Examples (What NOT to Do)

#### ❌ UnsafeCounter - No Synchronization
```cpp
class UnsafeCounter {
public:
    int count = 0;  // Public, unprotected
    void increment() { count++; }  // Race condition!
};
```
**Problem**: Multiple threads can read and write `count` simultaneously, causing lost updates.

#### ❌ LeakyStack - Leaking Data Handles
```cpp
class LeakyStack {
    std::vector<int> data;
public:
    std::vector<int>& getData() { 
        return data;  // DANGER: Returns reference to internal data
    }
};
```
**Problem**: External code can modify `data` without holding the mutex, bypassing all protection.

#### ❌ BadStack - Poor Interface Design
```cpp
bool isEmpty();  // Check
int top();       // Action
void pop();      // Another action
```
**Problem**: TOCTOU (Time-Of-Check-Time-Of-Use) bug. Another thread might modify the stack between `isEmpty()` and `top()` calls.

### Good Examples (Best Practices)

#### ✅ SafeStack - Proper Design
```cpp
class SafeStack {
private:
    std::mutex mtx;           // Principle 1: Mutex for synchronization
    std::vector<int> data;    // Principle 2: Private data (no leaks)
    
public:
    // Principle 3: Atomic check-and-action
    bool tryPop(int& result) {
        std::lock_guard<std::mutex> lock(mtx);
        if (data.empty()) return false;
        result = data.back();
        data.pop_back();
        return true;  // Check and action combined atomically
    }
    
    // Returns COPY, not reference
    std::vector<int> getAllData() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data;  // Safe: returns a copy
    }
};
```

#### ✅ SafeLogger - Encapsulated Resource
```cpp
class SafeLogger {
private:
    mutable std::mutex mtx;
    std::ofstream logFile;  // Private, protected by mtx
    
public:
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        logFile << message << std::endl;
    }
    
    // Deleted copy operations prevent accidental misuse
    SafeLogger(const SafeLogger&) = delete;
    SafeLogger& operator=(const SafeLogger&) = delete;
};
```

#### ✅ SafeCounter - Complete Protection
```cpp
class SafeCounter {
private:
    mutable std::mutex mtx;
    int count;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
    }
    
    int getCount() const {
        std::lock_guard<std::mutex> lock(mtx);
        return count;  // Returns copy, not reference
    }
};
```

## Demonstrations

### Demo 1: Race Condition with Unsafe Counter
Two threads increment a counter 10,000 times each. Expected result: 20,000. Actual result: unpredictable (often less due to lost updates).

**Output**:
```
Expected count: 20000
Actual count: 17843
Race condition likely caused incorrect result!
```

### Demo 2: Data Leak Danger
Shows how returning a reference to internal data allows external code to modify it without mutex protection.

### Demo 3: Bad Interface Design (TOCTOU Bug)
Demonstrates the Time-Of-Check-Time-Of-Use vulnerability:
```cpp
if (!stack.isEmpty()) {  // Thread A checks
    // Thread B might pop here!
    value = stack.top(); // Thread A crashes
}
```

### Demo 4: Safe Stack
Multiple producer and consumer threads safely interact with the stack using atomic operations.

### Demo 5: Safe Logger
Three threads write to the same log file concurrently without corruption.

**Output in thread_safe.log**:
```
[140234567890] Message 0 from thread 1
[140234567891] Message 0 from thread 2
[140234567892] Message 0 from thread 3
...
```

### Demo 6: Safe Counter
Two threads increment a counter 10,000 times each. Result: always exactly 20,000.

**Output**:
```
Expected count: 20000
Actual count: 20000
Perfect! No race condition.
```

## Building and Running

### Compilation
```bash
g++ -std=c++11 -pthread demo_005.cpp -o data_race_demo
```

### Execution
```bash
./data_race_demo
```

### Check Log File
```bash
cat thread_safe.log
```

## Key Concepts

### Data Race
Occurs when:
1. Two or more threads access the same memory location concurrently
2. At least one access is a write
3. There is no synchronization

**Result**: Undefined behavior, corrupted data, crashes.

### TOCTOU (Time-Of-Check-Time-Of-Use)
A race condition where the state changes between checking a condition and acting on it.

**Example**:
```cpp
// Bad: Check and use are separate
if (!isEmpty()) {      // Check at time T1
    value = top();     // Use at time T2 (state might have changed!)
}

// Good: Check and use are atomic
if (tryPop(value)) {   // Check and use happen together
    // Use value
}
```

### Handle Leaking
Returning references or pointers to internal data that should be protected by a mutex.

**Why it's dangerous**:
```cpp
std::vector<int>& data = obj.getData();  // Gets internal reference
// Now 'data' can be modified without holding obj's mutex!
data.push_back(42);  // DANGER: No mutex protection
```

### RAII (Resource Acquisition Is Initialization)
Using `std::lock_guard` ensures mutex is always released, even during exceptions.

## Best Practices Checklist

### ✅ Always Do
- [ ] Protect all shared data with mutexes
- [ ] Use `std::lock_guard` or `std::unique_lock` (RAII)
- [ ] Keep data members private
- [ ] Return copies, not references to internal data
- [ ] Combine check-and-action into single operations
- [ ] Make mutexes `mutable` for const methods that need to lock
- [ ] Delete copy constructors for classes with mutexes

### ❌ Never Do
- [ ] Access shared data without holding a lock
- [ ] Return references/pointers to protected data
- [ ] Manually lock/unlock (use RAII instead)
- [ ] Design interfaces that require multiple calls (TOCTOU risk)
- [ ] Lock in one function and unlock in another
- [ ] Hold locks longer than necessary

## Common Patterns

### Pattern 1: Thread-Safe Getter (Return Copy)
```cpp
Type getValue() const {
    std::lock_guard<std::mutex> lock(mtx);
    return internalValue;  // Returns a copy
}
```

### Pattern 2: Atomic Check-and-Action
```cpp
bool tryOperation(Result& output) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!canPerformOperation()) {
        return false;
    }
    output = performOperation();
    return true;
}
```

### Pattern 3: Exception-Safe Modification
```cpp
void modify(const Input& input) {
    std::lock_guard<std::mutex> lock(mtx);
    // All operations here are protected
    // Lock automatically released even if exception occurs
}
```

### Pattern 4: Encapsulated Resource
```cpp
class ThreadSafeResource {
private:
    std::mutex mtx;
    Resource resource;  // Never exposed
public:
    void safeOperation() {
        std::lock_guard<std::mutex> lock(mtx);
        resource.doSomething();
    }
};
```

## Comparison Table

| Aspect | Bad Practice | Good Practice |
|--------|-------------|---------------|
| **Synchronization** | No mutex | Mutex on all accesses |
| **Data Access** | Public members | Private members only |
| **Return Values** | References/pointers | Copies or smart pointers |
| **Interface** | Separate check/action | Atomic operations |
| **Lock Management** | Manual lock/unlock | RAII (lock_guard) |
| **Copy Operations** | Default copy | Deleted for mutex classes |

## Advanced Topics

### When to Use const and mutable
```cpp
class ThreadSafe {
    mutable std::mutex mtx;  // Can be locked in const methods
    int data;
    
public:
    int getData() const {  // const method
        std::lock_guard<std::mutex> lock(mtx);  // OK: mtx is mutable
        return data;
    }
};
```

### Why Delete Copy Operations
```cpp
class Logger {
    std::mutex mtx;  // Mutexes are not copyable
    
    // Explicitly delete copy operations
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
```
Prevents accidental copying which would break mutex semantics.

### Returning Smart Pointers (Advanced)
If you must return a pointer:
```cpp
std::shared_ptr<const Data> getData() const {
    std::lock_guard<std::mutex> lock(mtx);
    return std::make_shared<Data>(internalData);  // Returns copy as smart pointer
}
```

## Common Mistakes and Fixes

### Mistake 1: Returning Reference
```cpp
// ❌ Bad
const std::vector<int>& getData() {
    return data;  // Leaks handle!
}

// ✅ Good
std::vector<int> getData() const {
    std::lock_guard<std::mutex> lock(mtx);
    return data;  // Returns copy
}
```

### Mistake 2: Split Operations
```cpp
// ❌ Bad - TOCTOU bug
if (!queue.empty()) {       // Thread A checks
    value = queue.front();  // Thread B might clear queue here
    queue.pop();            // Crash or wrong value!
}

// ✅ Good - Atomic operation
if (queue.tryPop(value)) {
    // value is safely obtained
}
```

### Mistake 3: Inconsistent Locking
```cpp
// ❌ Bad - Some methods don't lock
void setValue(int v) { value = v; }  // No lock!
int getValue() { 
    std::lock_guard<std::mutex> lock(mtx);
    return value; 
}

// ✅ Good - All methods lock
void setValue(int v) { 
    std::lock_guard<std::mutex> lock(mtx);
    value = v; 
}
```

## Learning Objectives

This program teaches:
- The three principles of data race prevention
- How to identify race conditions
- Proper mutex usage with RAII
- Dangers of leaking data handles
- TOCTOU vulnerabilities and solutions
- Thread-safe class design patterns
- When and why to delete copy operations
- Exception-safe synchronization

## Requirements

- C++11 or later
- POSIX threads library (linked with `-pthread`)
- Compiler: GCC, Clang, or MSVC with C++11 support

## Conclusion

Data races are one of the most challenging bugs to find and fix. By following these three principles religiously, you can write thread-safe code that is robust, maintainable, and correct. Always remember:

1. **Synchronize**: Use mutexes for all shared data access
2. **Encapsulate**: Never leak handles to internal data
3. **Design Atomically**: Combine related operations

These principles are not optional—they are essential for correct multithreaded programming.


# 6. Deadlock Prevention [demo_006.cpp]

## Overview

This program demonstrates the **deadlock problem** in multithreaded programming and provides three different solutions. Deadlock is one of the most serious bugs in concurrent programming because it causes programs to hang indefinitely, requiring manual termination.

## What is Deadlock?

**Deadlock** occurs when two or more threads are waiting for each other to release resources, creating a circular dependency where no thread can proceed.

### Classic Deadlock Scenario

```
Thread A:                    Thread B:
1. Locks mutex1              1. Locks mutex2
2. Tries to lock mutex2 →    2. Tries to lock mutex1 →
   (WAITS forever)              (WAITS forever)
```

Both threads are stuck waiting for resources held by the other. The program **hangs** and never recovers without manual intervention (Ctrl+C).

### Real-World Analogy

Two people need two tools:
- **Person A** picks up a hammer and needs a screwdriver
- **Person B** picks up a screwdriver and needs a hammer
- Both wait forever for the other to release their tool

---

## The Four Conditions for Deadlock

Deadlock requires **all four** of these conditions (Coffman conditions):

1. **Mutual Exclusion**: Resources can only be held by one thread at a time
2. **Hold and Wait**: Threads hold resources while waiting for others
3. **No Preemption**: Resources can't be forcibly taken from threads
4. **Circular Wait**: Threads form a cycle of waiting (A waits for B, B waits for A)

**To prevent deadlock, break ANY ONE of these conditions.**

---

## Code Structure

The program demonstrates four logger implementations:

| Class | Description | Deadlock Risk |
|-------|-------------|---------------|
| `Logger1` | Inconsistent lock ordering | ❌ **WILL DEADLOCK** |
| `Logger2` | Consistent lock ordering | ✅ Safe |
| `Logger3` | Using `std::lock()` | ✅ Safe |
| `Logger4` | Buggy scoped locking | ⚠️ Undefined behavior |

---

## Examples

### ❌ Logger1: Deadlock Scenario

```cpp
class Logger1 {
    std::mutex mtx;
    std::mutex mtx2;
    
public:
    void log(std::string s) {
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx FIRST
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 SECOND
        std::cout << s << std::endl;
    }
    
    void log2(std::string s) {
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 FIRST
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx SECOND
        std::cout << s << std::endl;
    }
};
```

**Why it deadlocks:**

```
Time  Thread A (calls log())     Thread B (calls log2())
-----------------------------------------------------------
T1    Acquires mtx               Acquires mtx2
T2    Tries to acquire mtx2 →    Tries to acquire mtx →
      WAITS (mtx2 held by B)     WAITS (mtx held by A)
T3    ⏳ DEADLOCK ⏳             ⏳ DEADLOCK ⏳
```

**Circular wait**: A waits for B's mtx2, B waits for A's mtx.

### ✅ Logger2: Consistent Lock Ordering

```cpp
class Logger2 {
    std::mutex mtx;
    std::mutex mtx2;
    
public:
    void log(std::string s) {
        std::lock_guard<std::mutex> lock(mtx);   // mtx FIRST
        std::lock_guard<std::mutex> lock2(mtx2); // mtx2 SECOND
        std::cout << s << std::endl;
    }
    
    void log2(std::string s) {
        std::lock_guard<std::mutex> lock(mtx);   // mtx FIRST (same order!)
        std::lock_guard<std::mutex> lock2(mtx2); // mtx2 SECOND (same order!)
        std::cout << s << std::endl;
    }
};
```

**Why it works:**

Both methods lock mutexes in the **same order** (mtx → mtx2). This breaks the **circular wait** condition.

```
Time  Thread A                   Thread B
-----------------------------------------------------------
T1    Acquires mtx               Tries to acquire mtx → WAITS
T2    Acquires mtx2              Still waiting...
T3    Does work, releases both   Now acquires mtx
T4    Done                       Acquires mtx2, does work
```

No circular dependency → no deadlock.

### ✅ Logger3: Using std::lock()

```cpp
class Logger3 {
    std::mutex mtx;
    std::mutex mtx2;
    
public:
    void log(std::string s) {
        std::lock(mtx, mtx2);  // Locks BOTH atomically
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        std::cout << s << std::endl;
    }
    
    void log2(std::string s) {
        std::lock(mtx, mtx2);  // Locks BOTH atomically (order doesn't matter)
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        std::cout << s << std::endl;
    }
};
```

**Why it works:**

`std::lock()` uses a **deadlock-avoidance algorithm** to lock multiple mutexes atomically. It locks all mutexes or none at all.

**How std::lock() works:**
1. Try to lock all mutexes
2. If any lock fails, release all already-acquired locks
3. Retry with a different strategy
4. Guarantees deadlock-free acquisition

**Key points:**
- Order of mutexes in `std::lock()` doesn't matter
- `std::adopt_lock` tells `lock_guard` that the mutex is already locked
- `lock_guard` only manages the unlocking (RAII)

### ⚠️ Logger4: Buggy Implementation

```cpp
void log2(std::string s) {
    {
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        // BUG: mtx2 was never locked before this!
    }
    {
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        // BUG: mtx was never locked before this!
    }
    std::cout << s << std::endl;
}
```

**Problem:**

`std::adopt_lock` tells `lock_guard` "the mutex is already locked, just manage unlocking it." But these mutexes were **never locked** in the first place!

**Result:** Undefined behavior - trying to unlock mutexes that aren't locked.

**Correct usage:**
```cpp
std::lock(mtx, mtx2);  // Actually lock them first
std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);   // Now adopt is OK
std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock); // Now adopt is OK
```

---

## Prevention Techniques

### Technique 1: Use One Mutex (Best)

```cpp
class SimpleLogger {
    std::mutex mtx;  // Only one mutex
    std::ofstream f;
public:
    void log(std::string s) {
        std::lock_guard<std::mutex> lock(mtx);
        f << s << std::endl;
    }
};
```

**Advantage:** Can't deadlock with only one mutex!

### Technique 2: Consistent Lock Ordering

```cpp
// Always lock in the same order across all functions
void func1() {
    std::lock_guard<std::mutex> lock1(mtx1);  // mtx1 first
    std::lock_guard<std::mutex> lock2(mtx2);  // mtx2 second
}

void func2() {
    std::lock_guard<std::mutex> lock1(mtx1);  // mtx1 first (same order!)
    std::lock_guard<std::mutex> lock2(mtx2);  // mtx2 second (same order!)
}
```

**Rule:** Establish a global ordering of mutexes and always follow it.

### Technique 3: Use std::lock()

```cpp
// For locking multiple mutexes
std::lock(mtx1, mtx2, mtx3);  // Locks all atomically
std::lock_guard<std::mutex> lock1(mtx1, std::adopt_lock);
std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
std::lock_guard<std::mutex> lock3(mtx3, std::adopt_lock);
```

**Advantage:** Order doesn't matter - deadlock-free algorithm.

### Technique 4: Use std::scoped_lock (C++17)

```cpp
// C++17: Combines std::lock() and lock_guard
std::scoped_lock lock(mtx1, mtx2, mtx3);  // One line, deadlock-free!
```

**Advantage:** Simplest and most modern approach.

### Technique 5: Minimize Lock Scope

```cpp
// ❌ Bad: Locks held too long
std::lock_guard<std::mutex> lock1(mtx1);
std::lock_guard<std::mutex> lock2(mtx2);
expensive_operation();  // Holding both locks during expensive work

// ✅ Good: Lock, work, unlock
{
    std::lock_guard<std::mutex> lock1(mtx1);
    quick_operation1();
}
{
    std::lock_guard<std::mutex> lock2(mtx2);
    quick_operation2();
}
expensive_operation();  // No locks held
```

---

## Demonstrations

### Demo 1: Deadlock (Will Hang!)

**What happens:**
```bash
$ ./deadlock_demo
=== DEMO 1: Deadlock Scenario (WILL HANG!) ===
Thread A calls log()  -> locks mtx, waits for mtx2
Thread B calls log2() -> locks mtx2, waits for mtx
Result: DEADLOCK - both threads wait forever
Press Ctrl+C to terminate if it hangs...
[Program hangs here - use Ctrl+C to exit]
```

**Why:** Main thread calls `log2()` (locks mtx2→mtx), spawned thread calls `log()` (locks mtx→mtx2). Opposite orders create deadlock.

### Demo 2: Consistent Ordering (Safe)

**What happens:**
```bash
$ ./deadlock_demo
=== DEMO 2: Consistent Lock Ordering ===
Both threads lock mutexes in the same order
Result: NO DEADLOCK
T1 ---
--- main
T1 ---
--- main
[...continues until completion...]
Demo 2 completed successfully!
```

**Why:** Both threads lock in the same order (mtx→mtx2), preventing circular wait.

### Demo 3: Using std::lock() (Safe)

**What happens:**
```bash
$ ./deadlock_demo
=== DEMO 3: Using std::lock() ===
std::lock() locks multiple mutexes atomically
Lock order in code doesn't matter
Result: NO DEADLOCK
T1 ---
--- main
T1 ---
[...continues until completion...]
Demo 3 completed successfully!
```

**Why:** `std::lock()` uses a deadlock-avoidance algorithm.

### Demo 4: Buggy Implementation

**What happens:**
```bash
$ ./deadlock_demo
=== DEMO 4: Buggy Scoped Locking ===
WARNING: log2() has bugs with std::adopt_lock
This example shows what NOT to do
[May crash or exhibit undefined behavior]
```

**Why:** Uses `std::adopt_lock` without actually locking first - undefined behavior.

---

## Building and Running

### Compilation

```bash
g++ -std=c++11 -pthread demo_006.cpp -o deadlock_demo
```

### Running

```bash
./deadlock_demo
```

### Testing Specific Demos

Edit `main()` to uncomment the demo you want:

```cpp
int main() {
    // demo1();  // WARNING: Will hang!
    demo2();     // Safe
    // demo3();  // Safe
    // demo4();  // Buggy
    return 0;
}
```

**Important:** Run `demo1()` in a terminal where you can easily press Ctrl+C to terminate!

---

## Best Practices

### ✅ Do These Things

#### 1. Use One Mutex When Possible
```cpp
class Safe {
    std::mutex mtx;  // Single mutex
    // All data protected by one mutex
};
```

#### 2. Establish Lock Ordering
```cpp
// Rule: Always lock A before B
void anyFunction() {
    std::lock_guard<std::mutex> lockA(mutexA);
    std::lock_guard<std::mutex> lockB(mutexB);
}
```

#### 3. Use std::lock() for Multiple Locks
```cpp
std::lock(m1, m2, m3);
std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
std::lock_guard<std::mutex> lock3(m3, std::adopt_lock);
```

#### 4. Use std::scoped_lock (C++17)
```cpp
std::scoped_lock lock(m1, m2, m3);  // Simplest!
```

#### 5. Document Lock Ordering
```cpp
// LOCK ORDER: accountMutex -> transactionMutex -> logMutex
// ALL functions must follow this order
```

#### 6. Minimize Lock Duration
```cpp
{
    std::lock_guard<std::mutex> lock(mtx);
    // Only critical section here
}  // Release early
// Non-critical work here
```

### ❌ Don't Do These Things

#### 1. Never Lock in Inconsistent Order
```cpp
// ❌ Bad: Different orders
void func1() { lock(A); lock(B); }
void func2() { lock(B); lock(A); }  // DEADLOCK RISK!
```

#### 2. Never Call External Functions While Holding Locks
```cpp
// ❌ Bad: Calls unknown code with lock held
std::lock_guard<std::mutex> lock(mtx);
userCallback();  // What if this tries to lock mtx?
```

#### 3. Never Use std::adopt_lock Without Locking First
```cpp
// ❌ Bad: adopt_lock without actual lock
std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);  // UB!

// ✅ Good: Lock first, then adopt
mtx.lock();
std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);  // OK
```

#### 4. Avoid Nested Locks When Possible
```cpp
// ❌ Risky: Nested locks
void outer() {
    std::lock_guard<std::mutex> lock(mtx);
    inner();  // What if inner() also locks?
}
```

---

## Lock Granularity

### Fine-Grained Locking

**Multiple mutexes protecting small pieces of data**

```cpp
class FineGrained {
    std::mutex mtx1;
    int data1;
    
    std::mutex mtx2;
    int data2;
    
public:
    void updateData1() { std::lock_guard<std::mutex> lock(mtx1); data1++; }
    void updateData2() { std::lock_guard<std::mutex> lock(mtx2); data2++; }
};
```

**Advantages:**
- ✅ Better parallelism (threads can work on different data simultaneously)
- ✅ Less contention

**Disadvantages:**
- ❌ More complex code
- ❌ Higher deadlock risk
- ❌ Harder to maintain

### Coarse-Grained Locking

**One mutex protecting all data**

```cpp
class CoarseGrained {
    std::mutex mtx;
    int data1;
    int data2;
    
public:
    void updateData1() { std::lock_guard<std::mutex> lock(mtx); data1++; }
    void updateData2() { std::lock_guard<std::mutex> lock(mtx); data2++; }
};
```

**Advantages:**
- ✅ Simpler code
- ✅ Lower deadlock risk
- ✅ Easier to maintain

**Disadvantages:**
- ❌ Less parallelism
- ❌ More contention

### Recommendation

**Start coarse, optimize if needed:**
1. Begin with **one mutex** per class (coarse-grained)
2. Profile your application
3. Only move to fine-grained if you find a bottleneck
4. "Premature optimization is the root of all evil" - Donald Knuth

---

## Common Patterns

### Pattern 1: std::lock() with RAII

```cpp
void transferMoney(Account& from, Account& to, int amount) {
    // Lock both account mutexes atomically
    std::lock(from.mutex, to.mutex);
    
    // Adopt the locks (already locked by std::lock)
    std::lock_guard<std::mutex> lock1(from.mutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(to.mutex, std::adopt_lock);
    
    from.balance -= amount;
    to.balance += amount;
    
    // Locks released automatically
}
```

### Pattern 2: std::scoped_lock (C++17)

```cpp
void transferMoney(Account& from, Account& to, int amount) {
    // One line - locks both, deadlock-free
    std::scoped_lock lock(from.mutex, to.mutex);
    
    from.balance -= amount;
    to.balance += amount;
}
```

### Pattern 3: Lock Hierarchies

```cpp
class HierarchicalMutex {
    std::mutex mtx;
    unsigned long hierarchy_level;
    
public:
    void lock() {
        check_hierarchy();  // Verify correct order
        mtx.lock();
    }
};
```

### Pattern 4: Try-Lock with Backoff

```cpp
bool tryTransfer(Account& from, Account& to, int amount) {
    std::unique_lock<std::mutex> lock1(from.mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(to.mutex, std::defer_lock);
    
    // Try to lock both
    if (std::try_lock(lock1, lock2) == -1) {
        // Success: both locked
        from.balance -= amount;
        to.balance += amount;
        return true;
    }
    return false;  // Failed, try again later
}
```

---

## Debugging Deadlocks

### Detection Techniques

#### 1. Thread Dump (Linux/Mac)
```bash
# While program is hung:
kill -QUIT <pid>  # Sends SIGQUIT, prints thread states
```

#### 2. GDB Debugger
```bash
gdb ./program
(gdb) run
# Wait for deadlock
^C  # Ctrl+C to break
(gdb) info threads
(gdb) thread <n>
(gdb) bt  # Backtrace to see where each thread is stuck
```

#### 3. Valgrind Helgrind
```bash
valgrind --tool=helgrind ./program
# Detects potential deadlocks and lock ordering issues
```

#### 4. ThreadSanitizer (TSan)
```bash
g++ -fsanitize=thread -g demo_006.cpp -o deadlock_demo
./deadlock_demo
# Reports data races and potential deadlocks
```

# 7. Shared Mutex (Reader-Writer Lock) [demo_007.cpp]

## Overview

This lesson demonstrates **shared_mutex** (also known as a **reader-writer lock**), a powerful synchronization primitive introduced in C++17 that allows multiple threads to read data concurrently while ensuring exclusive access for writes.

## Table of Contents

- [What is a Shared Mutex?](#what-is-a-shared-mutex)
- [The Problem This Code Has](#the-problem-this-code-has)
- [Lock Types Explained](#lock-types-explained)
- [How Shared Mutex Works](#how-shared-mutex-works)
- [Performance Comparison](#performance-comparison)
- [Building and Running](#building-and-running)
- [Expected Output](#expected-output)
- [Corrected Implementation](#corrected-implementation)
- [When to Use Shared Mutex](#when-to-use-shared-mutex)
- [Best Practices](#best-practices)
- [Common Use Cases](#common-use-cases)
- [Complete Example](#complete-example)

---

## What is a Shared Mutex?

A **shared_mutex** (C++17) allows two types of locks:

### 1. Shared Lock (Read Lock) 🔓
- **Multiple threads** can hold this lock simultaneously
- Used for read-only operations
- Created with `std::shared_lock<std::shared_mutex>`

### 2. Exclusive Lock (Write Lock) 🔒
- **Only ONE thread** can hold this lock at a time
- Blocks all other readers and writers
- Created with `std::lock_guard<std::shared_mutex>` or `std::unique_lock<std::shared_mutex>`

### Visual Representation

```
Regular Mutex (std::mutex):
T1: [LOCK]----work----[UNLOCK]
T2:           [WAIT]----[LOCK]----work----[UNLOCK]
T3:                          [WAIT]----[LOCK]----work----[UNLOCK]
Result: All threads serialize (slow)

Shared Mutex (std::shared_mutex):
           READERS                    WRITER           READERS
T1: [READ LOCK]--read--[UNLOCK]
T2: [READ LOCK]--read--[UNLOCK]
T3: [READ LOCK]--read--[UNLOCK]     [WAIT]
T4:                              [EXCLUSIVE]--write--[UNLOCK]
T5:                                                        [READ LOCK]--read
T6:                                                        [READ LOCK]--read
Result: Multiple concurrent readers, exclusive writers (fast!)
```

---

## The Problem This Code Has

### ⚠️ Critical Bug in Original Code

The original code has a **serious bug** that defeats the entire purpose of using `shared_mutex`:

```cpp
void read(int i) {
    lock_guard<shared_mutex> sl(sh_mutex);  // ❌ WRONG!
    cout << "Read thread " << i << " with shared lock" << endl;
}
```

**Problem:** Uses `lock_guard` which creates an **EXCLUSIVE** lock, not a shared lock!

**Result:**
- Only ONE reader can execute at a time
- Readers wait for each other unnecessarily
- No performance benefit over regular mutex
- Defeats the entire purpose of shared_mutex!

### ✅ Corrected Version

```cpp
void read(int i) {
    shared_lock<shared_mutex> sl(sh_mutex);  // ✅ CORRECT!
    cout << "Read thread " << i << " with shared lock" << endl;
}
```

**Result:**
- Multiple readers can execute simultaneously
- Massive performance improvement for read-heavy workloads
- Proper use of shared_mutex!

---

## Lock Types Explained

### Comparison Table

| Lock Type | Mutex Type | Access Mode | Concurrent Access | Use Case |
|-----------|------------|-------------|-------------------|----------|
| `lock_guard<shared_mutex>` | shared_mutex | Exclusive | NO (1 thread only) | **WRITE** operations |
| `unique_lock<shared_mutex>` | shared_mutex | Exclusive | NO (1 thread only) | **WRITE** operations (flexible) |
| `shared_lock<shared_mutex>` | shared_mutex | Shared | YES (multiple threads) | **READ** operations |
| `lock_guard<mutex>` | mutex | Exclusive | NO (1 thread only) | General synchronization |

### Lock Behavior Matrix

| Current Lock State | Can Acquire Shared Lock? | Can Acquire Exclusive Lock? |
|-------------------|-------------------------|----------------------------|
| **Unlocked** | ✅ Yes | ✅ Yes |
| **Shared Lock(s)** | ✅ Yes (multiple) | ❌ No (must wait) |
| **Exclusive Lock** | ❌ No (must wait) | ❌ No (must wait) |

---

## How Shared Mutex Works

### Example Timeline

```
Time  Thread Operation           Lock State           Who Can Proceed?
--------------------------------------------------------------------------------
T0    (idle)                     UNLOCKED            Anyone
T1    Reader1 acquires shared    SHARED(1)           Other readers
T2    Reader2 acquires shared    SHARED(2)           Other readers
T3    Reader3 acquires shared    SHARED(3)           Other readers
T4    Writer1 tries exclusive    SHARED(3), W:WAIT   Nobody (writer waits)
T5    Reader1 releases           SHARED(2), W:WAIT   Other readers
T6    Reader2 releases           SHARED(1), W:WAIT   Other readers
T7    Reader3 releases           UNLOCKED, W:WAIT    Writer proceeds
T8    Writer1 acquires exclusive EXCLUSIVE(W1)       Nobody
T9    Reader4 tries shared       EXCLUSIVE, R:WAIT   Nobody (reader waits)
T10   Writer1 releases           UNLOCKED, R:WAIT    Reader proceeds
T11   Reader4 acquires shared    SHARED(1)           Other readers
```

### Key Points

1. **Multiple readers** can hold shared locks simultaneously
2. **Writers block everyone** (exclusive access)
3. **New readers cannot join** while a writer is waiting (prevents writer starvation)
4. **Lock upgrades are not allowed** (shared → exclusive requires unlock → lock)

---

## Performance Comparison

### Scenario: 100 reads, 2 writes

#### With Exclusive Locks Only (Wrong - Original Code)

```cpp
void read(int i) {
    lock_guard<shared_mutex> lock(sh_mutex);  // Exclusive!
    // work
}
```

**Execution:**
```
R R R R R W W R R R ... (all serialized)
├─┼─┼─┼─┼─┼─┼─┼─┼─┤
Total time = 102 operations × time_per_operation
```

**Performance:** Sequential, slow

#### With Shared Locks (Correct)

```cpp
void read(int i) {
    shared_lock<shared_mutex> lock(sh_mutex);  // Shared!
    // work
}
```

**Execution:**
```
RRRRR...  W  W  RRRRR... (readers parallel)
└────┤    ├  ├  └────┤
Parallel  Seq Seq Parallel

Total time ≈ read_time + (2 × write_time)
```

**Performance:** Massively parallel, fast

### Speedup Calculation

```
Speedup = (Sequential Time) / (Parallel Time)
        = (100 × 10ms + 2 × 100ms) / (10ms + 2 × 100ms)
        = 1200ms / 210ms
        = 5.7x faster!
```

For **1000 reads and 2 writes**, speedup approaches **10x or more**!

---

## Building and Running

### Prerequisites

- C++17 or later (for `std::shared_mutex`)
- Compiler: GCC 7+, Clang 5+, or MSVC 2017+

### Compilation

```bash
# GCC/Clang
g++ -std=c++17 -pthread demo_007.cpp -o shared_mutex_demo

# With optimizations
g++ -std=c++17 -pthread -O2 demo_007.cpp -o shared_mutex_demo
```

### Execution

```bash
./shared_mutex_demo
```

---

## Expected Output

### Original Version (Buggy)

```
=== ORIGINAL VERSION (Bug: readers use exclusive lock) ===
Watch: Readers will wait for each other (BAD!)
Expected behavior: All operations serialize (slow)

Read thread 0 with shared lock
Read thread 1 with shared lock
Read thread 2 with shared lock
Read thread 3 with shared lock
Read thread 4 with shared lock
print thread 5 with exclusive lock
(2 second pause)
print thread 6 with exclusive lock
(2 second pause)
Read thread 7 with shared lock
Read thread 8 with shared lock
... (all serialize, one at a time)
```

**Problem:** Notice how readers execute one at a time, not concurrently.

### Corrected Version

```
=== CORRECTED VERSION (Readers use shared lock) ===
Watch: Multiple readers execute concurrently (GOOD!)
Expected behavior: Readers run in parallel, writers get exclusive access

READER thread 0 - shared access
READER thread 1 - shared access
READER thread 2 - shared access
READER thread 3 - shared access
READER thread 4 - shared access
(All 5 readers execute together!)
READER thread 0 - finished reading
READER thread 1 - finished reading
READER thread 2 - finished reading
READER thread 3 - finished reading
READER thread 4 - finished reading
WRITER thread 5 - exclusive access
(500ms pause - exclusive access)
WRITER thread 5 - finished writing
WRITER thread 6 - exclusive access
(500ms pause - exclusive access)
WRITER thread 6 - finished writing
READER thread 7 - shared access
READER thread 8 - shared access
... (multiple readers again!)
```

**Benefit:** Readers execute concurrently, completing much faster!

---

## Corrected Implementation

### Complete Thread-Safe Class Example

```cpp
#include <shared_mutex>
#include <map>
#include <string>

class ThreadSafeCache {
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, int> cache_;
    
public:
    // READ operation - uses SHARED lock
    int get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
        return -1;  // Not found
    }
    
    // READ operation - multiple readers can call this
    bool contains(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_.find(key) != cache_.end();
    }
    
    // WRITE operation - uses EXCLUSIVE lock
    void set(const std::string& key, int value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_[key] = value;
    }
    
    // WRITE operation - exclusive access
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_.clear();
    }
    
    // READ operation - returns copy
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_.size();
    }
};
```

### Usage Example

```cpp
ThreadSafeCache cache;

// Multiple reader threads (can run concurrently)
auto reader = [&cache](int id) {
    for (int i = 0; i < 1000; ++i) {
        int value = cache.get("key" + std::to_string(i % 10));
        // Process value...
    }
};

// Writer thread (exclusive access)
auto writer = [&cache](int id) {
    for (int i = 0; i < 100; ++i) {
        cache.set("key" + std::to_string(i), i * 10);
    }
};

// Launch threads
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread(reader, i));  // 10 readers
}
threads.push_back(std::thread(writer, 0));      // 1 writer

for (auto& t : threads) {
    t.join();
}
```

---

## When to Use Shared Mutex

### ✅ Use shared_mutex When:

1. **Read-Heavy Workloads**
   ```
   Ratio: 90% reads, 10% writes or better
   Example: Configuration cache, lookup tables
   ```

2. **Expensive Read Operations**
   ```
   Read operations take significant time
   Example: Parsing cached data, complex calculations
   ```

3. **High Contention on Reads**
   ```
   Many threads need to read simultaneously
   Example: Reference data accessed by all threads
   ```

4. **Rare Writes**
   ```
   Writes happen infrequently
   Example: Config reload, cache invalidation
   ```

### ❌ Don't Use shared_mutex When:

1. **Write-Heavy Workloads**
   ```
   Ratio: 50% writes or more
   Use: Regular std::mutex (simpler, less overhead)
   ```

2. **Very Short Critical Sections**
   ```
   Operations complete in microseconds
   Use: std::mutex or std::atomic (less overhead)
   ```

3. **Equal Read/Write Mix**
   ```
   Reads and writes are equally common
   Use: Regular std::mutex
   ```

4. **Single-Threaded Reads**
   ```
   Only one thread reads at a time anyway
   Use: Regular std::mutex (simpler)
   ```

### Performance Threshold

```
shared_mutex overhead ≈ 2-3x regular mutex overhead

Break-even point:
- Need at least 2-3 concurrent readers to justify overhead
- Read/Write ratio should be at least 80/20
- Critical section should be non-trivial (> 100 instructions)

Always profile before and after!
```

---

## Best Practices

### ✅ Do These Things

#### 1. Use Correct Lock Types
```cpp
// ✅ CORRECT
void read() {
    std::shared_lock<std::shared_mutex> lock(mtx);  // Shared
    // read-only operations
}

void write() {
    std::unique_lock<std::shared_mutex> lock(mtx);  // Exclusive
    // write operations
}
```

#### 2. Keep Critical Sections Small
```cpp
// ✅ GOOD: Lock only around shared data access
void processData() {
    Data localCopy;
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        localCopy = sharedData;  // Quick copy
    }  // Lock released here
    
    expensiveProcessing(localCopy);  // No lock held
}
```

#### 3. Make Mutex Mutable for Const Methods
```cpp
class Cache {
    mutable std::shared_mutex mtx_;  // mutable!
    std::map<int, std::string> data_;
    
public:
    std::string get(int key) const {  // const method
        std::shared_lock<std::shared_mutex> lock(mtx_);
        // Can lock because mtx_ is mutable
        return data_.at(key);
    }
};
```

#### 4. Document Lock Requirements
```cpp
class DataStore {
    // All methods use shared_lock for reads, unique_lock for writes
    // Lock hierarchy: Always acquire locks in ID order if multiple stores
    std::shared_mutex mtx_;
    
public:
    // REQUIRES: No locks held
    // LOCKS: shared_mutex (shared)
    std::string read(int id) const;
    
    // REQUIRES: No locks held
    // LOCKS: shared_mutex (exclusive)
    void write(int id, const std::string& value);
};
```

#### 5. Prefer shared_lock for Const Methods
```cpp
class ThreadSafeContainer {
    mutable std::shared_mutex mtx_;
    std::vector<int> data_;
    
public:
    // All const methods use shared_lock
    size_t size() const {
        std::shared_lock lock(mtx_);
        return data_.size();
    }
    
    bool empty() const {
        std::shared_lock lock(mtx_);
        return data_.empty();
    }
    
    // Non-const methods use unique_lock
    void push_back(int val) {
        std::unique_lock lock(mtx_);
        data_.push_back(val);
    }
};
```

### ❌ Don't Do These Things

#### 1. Don't Use Exclusive Lock for Reads
```cpp
// ❌ BAD: Defeats purpose of shared_mutex
void read() {
    std::lock_guard<std::shared_mutex> lock(mtx);  // Exclusive!
    // read operation
}
```

#### 2. Don't Hold Locks During I/O
```cpp
// ❌ BAD: Holds lock during slow I/O
void logData() {
    std::shared_lock<std::shared_mutex> lock(mtx);
    std::cout << data_ << std::endl;  // Slow I/O with lock held!
}

// ✅ GOOD: Copy data, then log
void logData() {
    std::string copy;
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        copy = data_;
    }
    std::cout << copy << std::endl;  // I/O without lock
}
```

#### 3. Don't Try to Upgrade Locks
```cpp
// ❌ BAD: Can't upgrade shared → exclusive
void conditionalWrite() {
    std::shared_lock<std::shared_mutex> read_lock(mtx);
    if (needsUpdate(data_)) {
        read_lock.unlock();
        std::unique_lock<std::shared_mutex> write_lock(mtx);  // Race condition!
        data_ = newValue;
    }
}

// ✅ GOOD: Check, unlock, then exclusive lock
void conditionalWrite() {
    bool needs_update;
    {
        std::shared_lock lock(mtx);
        needs_update = needsUpdate(data_);
    }
    if (needs_update) {
        std::unique_lock lock(mtx);
        // Re-check condition!
        if (needsUpdate(data_)) {
            data_ = newValue;
        }
    }
}
```

#### 4. Don't Mix with Regular Mutex
```cpp
// ❌ BAD: Mixing lock types
std::mutex regular_mtx;
std::shared_mutex shared_mtx;

void badFunction() {
    std::lock_guard<std::mutex> lock1(regular_mtx);
    std::shared_lock<std::shared_mutex> lock2(shared_mtx);
    // Confusing and error-prone!
}
```

---

## Common Use Cases

### 1. Configuration Cache

```cpp
class ConfigCache {
    mutable std::shared_mutex mtx_;
    std::map<std::string, std::string> config_;
    
public:
    // Frequent: Many threads read config
    std::string get(const std::string& key) const {
        std::shared_lock lock(mtx_);
        return config_.at(key);
    }
    
    // Rare: Config reload
    void reload(const std::map<std::string, std::string>& new_config) {
        std::unique_lock lock(mtx_);
        config_ = new_config;
    }
};
```

### 2. Database Query Cache

```cpp
class QueryCache {
    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, QueryResult> cache_;
    
public:
    // Frequent: Cache lookups
    std::optional<QueryResult> lookup(const std::string& query) const {
        std::shared_lock lock(mtx_);
        auto it = cache_.find(query);
        if (it != cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    // Less frequent: Cache updates
    void insert(const std::string& query, const QueryResult& result) {
        std::unique_lock lock(mtx_);
        cache_[query] = result;
    }
    
    // Rare: Cache invalidation
    void invalidate() {
        std::unique_lock lock(mtx_);
        cache_.clear();
    }
};
```

### 3. Thread-Safe Lookup Table

```cpp
class SymbolTable {
    mutable std::shared_mutex mtx_;
    std::map<std::string, int> symbols_;
    
public:
    // Frequent: Symbol lookups
    int lookup(const std::string& name) const {
        std::shared_lock lock(mtx_);
        return symbols_.at(name);
    }
    
    // Less frequent: Symbol definition
    void define(const std::string& name, int value) {
        std::unique_lock lock(mtx_);
        symbols_[name] = value;
    }
};
```

### 4. Observer Pattern

```cpp
class Subject {
    mutable std::shared_mutex mtx_;
    std::vector<Observer*> observers_;
    
public:
    // Frequent: Notify all observers
    void notify() const {
        std::shared_lock lock(mtx_);
        for (auto* obs : observers_) {
            obs->update();  // Readers can iterate concurrently
        }
    }
    
    // Rare: Add observer
    void attach(Observer* obs) {
        std::unique_lock lock(mtx_);
        observers_.push_back(obs);
    }
    
    // Rare: Remove observer
    void detach(Observer* obs) {
        std::unique_lock lock(mtx_);
        observers_.erase(std::remove(observers_.begin(), 
                                     observers_.end(), obs), 
                        observers_.end());
    }
};
```

---

## Complete Example

Here's a complete, production-ready example:

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <map>
#include <chrono>
#include <random>

class ThreadSafeDictionary {
private:
    mutable std::shared_mutex mtx_;
    std::map<std::string, int> data_;
    
public:
    // READ operation
    int get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : -1;
    }
    
    // WRITE operation
    void set(const std::string& key, int value) {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        data_[key] = value;
    }
    
    // READ operation
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        return data_.size();
    }
};

int main() {
    ThreadSafeDictionary dict;
    std::vector<std::thread> threads;
    
    // Create 10 reader threads
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&dict, i]() {
            for (int j = 0; j < 100; ++j) {
                int value = dict.get("key" + std::to_string(j % 10));
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            std::cout << "Reader " << i << " completed\n";
        });
    }
    
    // Create 2 writer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&dict, i]() {
            for (int j = 0; j < 10; ++j) {
                dict.set("key" + std::to_string(j), j * 100);
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "Writer " << i << " completed\n";
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final dictionary size: " << dict.size() << "\n";
    
    return 0;
}
```

---

## Key Takeaways

1. **`shared_lock`** for reads (multiple concurrent)
2. **`unique_lock`** or **`lock_guard`** for writes (exclusive)
3. Use **shared_mutex** only for **read-heavy** workloads
4. Always **profile** before and after
5. **Encapsulate** shared_mutex with the data it protects
6. **Document** your locking strategy
7. Keep **critical sections small**

---

## Requirements

- **C++17** or later for `std::shared_mutex`
- **POSIX threads** library (`-pthread`)
- Compiler: GCC 7+, Clang 5+, MSVC 2017+

## Further Reading

- [C++ Reference: std::shared_mutex](https://en.cppreference.com/w/cpp/thread/shared_mutex)
- [C++ Reference: std::shared_lock](https://en.cppreference.com/w/cpp/thread/shared_lock)
- Reader-Writer Problem
- Lock-Free Programming (advanced alternative)