#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

// Type alias for cleaner code
typedef std::mutex mutex_t;

// Global mutex for protecting shared resources (std::cout)
// Multiple threads will use this to synchronize access
mutex_t mtx;

// Method 1: Manual mutex locking and unlocking
// Simple but error-prone - must remember to unlock
void dispMessage1(std::string s)
{
    mtx.lock();                      // Acquire the lock
    std::cout << s << std::endl;     // Protected operation
    mtx.unlock();                    // Release the lock
    // DANGER: If an exception occurs between lock() and unlock(),
    // the mutex will never be unlocked (deadlock risk)
}

// Method 2: RAII-based mutex management (RECOMMENDED)
// Uses std::lock_guard for automatic lock/unlock
void dispMessage2(std::string s)
{
    // RAII (Resource Acquisition Is Initialization)
    // Lock is acquired in constructor, released in destructor
    // Exception-safe: lock is released even if an exception occurs
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << s << std::endl;
    // Lock is automatically released when 'lock' goes out of scope
}

// Binding mutex to resource - BEST PRACTICE
// Encapsulates both the resource (file) and its protection mechanism (mutex)
// This ensures the mutex and resource are always used together
class Logger
{
    std::mutex mtx;      // Each Logger has its own mutex
    std::ofstream f;     // File stream to write logs

public:
    // Constructor: Opens the log file
    Logger()
    {
        f.open("app.log");
    }

    // Destructor: Ensures file is properly closed
    virtual ~Logger()
    {
        if(f.is_open()) f.close();  
    }

    // Thread-safe logging method
    // The mutex ensures only one thread can write at a time
    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        f << s << std::endl;
        // Lock is automatically released when method returns
    }
};

// DEMO 1: NO SYNCHRONIZATION
// Multiple threads writing to std::cout without protection
// Result: Garbled, interleaved output (race condition)
void func1()
{
    for (int i = 0; i < 100; i++)
    {
        std::cout << "T1 ---\n";  // UNSAFE: No mutex protection
    }
}

// DEMO 2: Manual mutex locking
// Uses dispMessage1() which manually locks/unlocks
void func2()
{
    for (int i = 0; i < 100; i++)
    {
        dispMessage1("T2 ---");  // Protected by manual lock/unlock
    }
}

// DEMO 3: RAII-based mutex locking (PREFERRED)
// Uses dispMessage2() which uses std::lock_guard
void func3()
{
    for (int i = 0; i < 100; i++)
    {
        dispMessage2("T2 ---");  // Protected by lock_guard (exception-safe)
    }
}

// DEMO 4: Resource-bound mutex pattern
// Uses Logger class which encapsulates mutex with the resource
void func4(Logger &logger)
{
    for (int i = 0; i < 100; i++)
    {
        logger.log("T1 ---");  // Thread-safe logging
    }
}

// DEMO 1: Race condition demonstration
// Both threads write to std::cout without synchronization
void demo1()
{
    std::thread t(func1);

    try
    {
        // Main thread also writes to std::cout
        for (int i = 0; i > -100; --i)
        {
            std::cout << "--- main\n";  // UNSAFE: No mutex protection
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    // Expected: Garbled output with mixed "T1 ---" and "--- main" lines
}

// DEMO 2: Manual mutex protection
// Both threads use dispMessage1() for synchronized output
void demo2()
{
    std::thread t(func2);

    try
    {
        for (int i = 0; i > -100; --i)
        {
            dispMessage1("--- main");  // Protected by manual lock
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    // Expected: Clean output, but vulnerable to exception-based deadlocks
}

// DEMO 3: RAII mutex protection (BEST for shared resources)
// Both threads use dispMessage2() with lock_guard
void demo3()
{
    std::thread t(func3);

    try
    {
        for (int i = 0; i > -100; --i)
        {
            dispMessage2("--- main");  // Protected by lock_guard
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
    // Expected: Clean output, exception-safe
}

// DEMO 4: Encapsulated mutex pattern (BEST for owned resources)
// Logger owns both the file and the mutex protecting it
void demo4(){
    Logger logger_;

    // Pass logger by reference to the thread
    std::thread t(func4, std::ref(logger_));

    try{
        for (int i = 0; i > -100; --i)
        {
            logger_.log("--- main");  // Thread-safe file writing
        }
    }
    catch(...){
        t.join();
        throw;
    }

    t.join();
    // Expected: Clean log file with all entries properly written
}

int main()
{
    // Run all four demonstrations sequentially
    // Each shows progressively better synchronization practices
    
    std::cout << "=== DEMO 1: No Synchronization (Race Condition) ===" << std::endl;
    demo1();
    
    std::cout << "\n=== DEMO 2: Manual Mutex Locking ===" << std::endl;
    demo2();
    
    std::cout << "\n=== DEMO 3: RAII Mutex (lock_guard) ===" << std::endl;
    demo3();
    
    std::cout << "\n=== DEMO 4: Encapsulated Mutex Pattern ===" << std::endl;
    demo4();
    std::cout << "Check app.log for demo4 output" << std::endl;

    return 0;
}