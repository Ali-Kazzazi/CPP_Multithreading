#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

// ============================================================================
// DEADLOCK DEMONSTRATION AND PREVENTION TECHNIQUES
// ============================================================================

// ============================================================================
// BAD EXAMPLE: Logger1 - DEADLOCK PRONE
// ============================================================================
// This class demonstrates a classic deadlock scenario
// Different methods lock mutexes in DIFFERENT ORDERS
class Logger1
{
    std::mutex mtx;   // First mutex
    std::mutex mtx2;  // Second mutex
    std::ofstream f;

public:
    Logger1()
    {
        f.open("app.log");
    }

    virtual ~Logger1()
    {
        if (f.is_open())
            f.close();
    }

    // Locks mtx THEN mtx2
    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx first
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 second
        std::cout << s << std::endl;
    }

    // DANGER: Locks mtx2 THEN mtx - OPPOSITE ORDER!
    // This creates a deadlock opportunity
    void log2(std::string s)
    {
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 first
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx second
        std::cout << s << std::endl;
    }
    
    // DEADLOCK SCENARIO:
    // Thread A calls log():  locks mtx, tries to lock mtx2
    // Thread B calls log2(): locks mtx2, tries to lock mtx
    // Both threads wait forever for each other (circular wait)
};

// ============================================================================
// SOLUTION 1: Logger2 - CONSISTENT LOCK ORDERING
// ============================================================================
// Prevention technique: Always lock mutexes in the SAME ORDER
class Logger2
{
    std::mutex mtx;
    std::mutex mtx2;
    std::ofstream f;

public:
    Logger2()
    {
        f.open("app.log");
    }

    virtual ~Logger2()
    {
        if (f.is_open())
            f.close();
    }

    // Locks mtx THEN mtx2
    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx first
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 second
        std::cout << s << std::endl;
    }

    // ALSO locks mtx THEN mtx2 - SAME ORDER!
    // This prevents deadlock
    void log2(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);   // Lock mtx first (same order)
        std::lock_guard<std::mutex> lock2(mtx2); // Lock mtx2 second (same order)
        std::cout << s << std::endl;
    }
    
    // NO DEADLOCK: All methods lock in the same order
    // Thread A and B can never be waiting for each other in a cycle
};

// ============================================================================
// SOLUTION 2: Logger3 - std::lock() FOR ATOMIC MULTI-LOCK
// ============================================================================
// Prevention technique: Use std::lock() to lock multiple mutexes atomically
// Order doesn't matter - std::lock() prevents deadlock
class Logger3
{
    std::mutex mtx;
    std::mutex mtx2;
    std::ofstream f;

public:
    Logger3()
    {
        f.open("app.log");
    }

    virtual ~Logger3()
    {
        if (f.is_open())
            f.close();
    }

    void log(std::string s)
    {
        // std::lock() locks BOTH mutexes atomically (deadlock-free algorithm)
        std::lock(mtx, mtx2);
        
        // std::adopt_lock tells lock_guard that mutex is ALREADY locked
        // lock_guard only manages unlocking (RAII cleanup)
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        std::cout << s << std::endl;
        
        // Both locks released when lock and lock2 go out of scope
    }

    void log2(std::string s)
    {
        // Lock both mutexes atomically
        std::lock(mtx, mtx2);
        
        // Notice: Different order of lock_guard creation
        // Doesn't matter because std::lock() already locked them
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        std::cout << s << std::endl;
    }
    
    // NO DEADLOCK: std::lock() uses a deadlock-avoidance algorithm
};

// ============================================================================
// SOLUTION 3: Logger4 - SCOPED LOCKING (NOT RECOMMENDED)
// ============================================================================
// This demonstrates manual scope control for locks
// WARNING: This example has a BUG - std::adopt_lock without actual locking!
class Logger4
{
    std::mutex mtx;
    std::mutex mtx2;
    std::ofstream f;

public:
    Logger4()
    {
        f.open("app.log");
    }

    virtual ~Logger4()
    {
        if (f.is_open())
            f.close();
    }

    void log(std::string s)
    {
        // Correct usage: std::lock() then adopt
        std::lock(mtx, mtx2);
        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        std::cout << s << std::endl;
    }

    void log2(std::string s)
    {
        // BUG: Using std::adopt_lock without actually locking first!
        // This code will cause undefined behavior
        {
            std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
            // BUG: mtx2 was never locked before this!
            // ...
            // ...
        } // mtx2 unlocked here

        {
            std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
            // BUG: mtx was never locked before this!
            // ...
            // ...
        } // mtx unlocked here
        
        std::cout << s << std::endl;
    }
    
    // NOTE: This example shows what NOT to do
    // std::adopt_lock should only be used after manually locking
};

// ============================================================================
// THREAD FUNCTIONS
// ============================================================================

// Thread function for Logger1 (deadlock-prone)
void func1(Logger1 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---"); // Locks mtx then mtx2
    }
}

// Thread function for Logger2 (consistent ordering)
void func2(Logger2 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---"); // Locks mtx then mtx2
    }
}

// Thread function for Logger3 (std::lock)
void func3(Logger3 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---"); // Uses std::lock()
    }
}

// Thread function for Logger4 (buggy scoped locking)
void func4(Logger4 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---"); // Uses std::lock() correctly
    }
}

// ============================================================================
// DEMONSTRATIONS
// ============================================================================

// DEMO 1: Deadlock occurs - program will hang!
void demo1()
{
    std::cout << "=== DEMO 1: Deadlock Scenario (WILL HANG!) ===" << std::endl;
    std::cout << "Thread A calls log()  -> locks mtx, waits for mtx2" << std::endl;
    std::cout << "Thread B calls log2() -> locks mtx2, waits for mtx" << std::endl;
    std::cout << "Result: DEADLOCK - both threads wait forever" << std::endl;
    std::cout << "Press Ctrl+C to terminate if it hangs..." << std::endl;
    
    Logger1 logger;
    std::thread t(func1, std::ref(logger));

    try
    {
        for (int i = 0; i > -1000; --i)
        {
            logger.log2("--- main"); // Main thread: locks mtx2 then mtx
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    std::cout << "Demo 1 completed (unlikely to reach here due to deadlock)" << std::endl;
}

// DEMO 2: No deadlock - consistent lock ordering
void demo2()
{
    std::cout << "\n=== DEMO 2: Consistent Lock Ordering ===" << std::endl;
    std::cout << "Both threads lock mutexes in the same order" << std::endl;
    std::cout << "Result: NO DEADLOCK" << std::endl;
    
    Logger2 logger;
    std::thread t(func2, std::ref(logger));

    try
    {
        for (int i = 0; i > -1000; --i)
        {
            logger.log2("--- main"); // Also locks mtx then mtx2
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    std::cout << "Demo 2 completed successfully!" << std::endl;
}

// DEMO 3: No deadlock - using std::lock()
void demo3()
{
    std::cout << "\n=== DEMO 3: Using std::lock() ===" << std::endl;
    std::cout << "std::lock() locks multiple mutexes atomically" << std::endl;
    std::cout << "Lock order in code doesn't matter" << std::endl;
    std::cout << "Result: NO DEADLOCK" << std::endl;
    
    Logger3 logger;
    std::thread t(func3, std::ref(logger));

    try
    {
        for (int i = 0; i > -1000; --i)
        {
            logger.log2("--- main"); // Uses std::lock() - order doesn't matter
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    std::cout << "Demo 3 completed successfully!" << std::endl;
}

// DEMO 4: Buggy example - demonstrates incorrect use of std::adopt_lock
void demo4()
{
    std::cout << "\n=== DEMO 4: Buggy Scoped Locking ===" << std::endl;
    std::cout << "WARNING: log2() has bugs with std::adopt_lock" << std::endl;
    std::cout << "This example shows what NOT to do" << std::endl;
    
    Logger4 logger;
    std::thread t(func4, std::ref(logger));

    try
    {
        for (int i = 0; i > -1000; --i)
        {
            logger.log("--- main"); // Uses correct method
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    std::cout << "Demo 4 completed (may have undefined behavior)" << std::endl;
}

// ============================================================================
// DEADLOCK PREVENTION ADVICE
// ============================================================================
/* 
BEST PRACTICES FOR AVOIDING DEADLOCK:

1. USE FEWER MUTEXES
   - Ideally, use only ONE mutex per class
   - Reduces complexity and deadlock opportunities

2. CONSISTENT LOCK ORDERING
   - If you must use multiple mutexes, ALWAYS lock them in the same order
   - Example: Always lock mtx1 before mtx2, never the reverse

3. USE std::lock() FOR MULTIPLE MUTEXES
   - std::lock(m1, m2, m3, ...) locks all atomically
   - Uses deadlock-avoidance algorithm
   - Order doesn't matter

4. MINIMIZE LOCK SCOPE
   - Only hold locks for the minimum time necessary
   - Release locks before calling external functions
   - Use scope blocks {} to control lock lifetime

5. AVOID NESTED LOCKS WHEN POSSIBLE
   - Lock one mutex, do work, unlock, then lock another
   - Only use nested locks when absolutely necessary

6. USE std::scoped_lock (C++17)
   - Combines std::lock() and std::lock_guard
   - Example: std::scoped_lock lock(mtx1, mtx2);
*/

// ============================================================================
// LOCK GRANULARITY TRADEOFFS
// ============================================================================
/*
FINE-GRAINED LOCKING:
- Multiple mutexes protecting small pieces of data
- Advantages:
  * Better parallelism (threads can work on different data simultaneously)
  * Less contention
- Disadvantages:
  * More complex code
  * Higher risk of deadlock
  * Harder to reason about correctness

COARSE-GRAINED LOCKING:
- One or few mutexes protecting large amounts of data
- Advantages:
  * Simpler code
  * Lower deadlock risk
  * Easier to reason about correctness
- Disadvantages:
  * Less parallelism (threads wait even when working on different data)
  * More contention
  * Potential performance bottleneck

RECOMMENDATION:
- Start with coarse-grained locking (one mutex)
- Only move to fine-grained if profiling shows it's a bottleneck
- "Premature optimization is the root of all evil"
*/

// ============================================================================
// MAIN
// ============================================================================
int main()
{
    std::cout << "=== DEADLOCK PREVENTION DEMONSTRATIONS ===" << std::endl;
    std::cout << "\nChoose which demo to run by uncommenting in main()" << std::endl;
    std::cout << "WARNING: demo1() will likely hang due to deadlock!" << std::endl;
    
    // Uncomment ONE demo at a time:
    
    // demo1();  // WARNING: Will deadlock! Press Ctrl+C to terminate
    // demo2();  // Safe: Consistent lock ordering
    // demo3();  // Safe: Using std::lock()
    demo4();     // Buggy: Shows incorrect use of adopt_lock
    
    std::cout << "\n=== DEMONSTRATION COMPLETE ===" << std::endl;
    
    return 0;
}