# C++ Multithreading

## Table of Contents

- [1. Thread implementation [demo_001.cpp]](#thread-implementation)
- [2. Sending Functors into Threads [demo_002.cpp]](#sending-functors-into-threads)
- [3. Thread Creation Methods [demo_003.cpp]](#thread-creation-methods)
- [4. Mutex and Thread Synchronization [demo_4.cpp]](#mutex-and-thread-synchronization)
- [5. Avoiding Data Race: Best Practices [demo_005.cpp]](#avoiding-data-race-best-practices)
- [6. Deadlock Prevention [demo_006.cpp]](#deadlock-prevention)


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




# Sending Functors into Threads

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




# Thread Creation Methods

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



# Mutex and Thread Synchronization

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
g++ -std=c++11 -pthread demo_04.cpp -o mutex_demo
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






# Avoiding Data Race: Best Practices

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
g++ -std=c++11 -pthread demo_006.cpp -o data_race_demo
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


# Deadlock Prevention

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
g++ -std=c++11 -pthread deadlock_demo.cpp -o deadlock_demo
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
g++ -fsanitize=thread -g deadlock_demo.cpp -o deadlock_demo
./deadlock_demo
# Reports data races and potential deadlocks
```

###