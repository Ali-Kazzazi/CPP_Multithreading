#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <fstream>

// ============================================================================
// BAD EXAMPLE 1: Data Race - No Synchronization
// ============================================================================
class UnsafeCounter {
public:
    int count = 0;  // Public member - accessible without protection
    
    void increment() {
        count++;  // NOT ATOMIC - race condition!
    }
    
    int getCount() {
        return count;  // Unprotected read
    }
};

// ============================================================================
// BAD EXAMPLE 2: Leaking Data Handles
// ============================================================================
class LeakyStack {
    std::mutex mtx;
    std::vector<int> data;
    
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push_back(value);
    }
    
    // BAD: Returns reference to internal data
    // External code can modify data without holding the mutex!
    std::vector<int>& getData() {
        return data;  // DANGER: Leaks internal data
    }
    
    // BAD: Returns pointer to internal data
    std::vector<int>* getDataPtr() {
        return &data;  // DANGER: Leaks internal data
    }
};

// ============================================================================
// BAD EXAMPLE 3: Poorly Designed Interface
// ============================================================================
class BadStack {
    std::mutex mtx;
    std::vector<int> data;
    
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push_back(value);
    }
    
    // BAD DESIGN: Check and action are separate
    // Race condition between isEmpty() and top() calls
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }
    
    int top() {
        std::lock_guard<std::mutex> lock(mtx);
        return data.back();  // May crash if empty
    }
    
    void pop() {
        std::lock_guard<std::mutex> lock(mtx);
        data.pop_back();
    }
};

// ============================================================================
// GOOD EXAMPLE: Thread-Safe Stack with Proper Design
// ============================================================================
class SafeStack {
private:
    // PRINCIPLE 1: Use mutex to synchronize all data access
    mutable std::mutex mtx;
    
    // PRINCIPLE 2: Never leak handles - keep data private
    std::vector<int> data;
    
public:
    // Thread-safe push operation
    void push(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push_back(value);
    }
    
    // PRINCIPLE 3: Design interface appropriately
    // Combine check and action into a single atomic operation
    // Returns false if stack is empty, true otherwise
    bool tryPop(int& result) {
        std::lock_guard<std::mutex> lock(mtx);
        if (data.empty()) {
            return false;  // Indicate failure
        }
        result = data.back();
        data.pop_back();
        return true;  // Indicate success
    }
    
    // Alternative: Pop and return the value
    // Throws exception if empty (clear contract)
    int pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (data.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        int result = data.back();
        data.pop_back();
        return result;
    }
    
    // Thread-safe size query
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.size();
    }
    
    // Thread-safe empty check
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }
    
    // GOOD: Return a COPY of data, not a reference
    // Caller gets their own copy, can't affect our internal state
    std::vector<int> getAllData() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data;  // Returns a copy
    }
};

// ============================================================================
// GOOD EXAMPLE: Thread-Safe Logger
// ============================================================================
class SafeLogger {
private:
    // Mutex and resource are encapsulated together
    mutable std::mutex mtx;  // mutable allows locking in const methods
    std::ofstream logFile;
    
public:
    SafeLogger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }
    
    ~SafeLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    // Delete copy constructor and assignment operator
    // Prevents copying which would break mutex semantics
    SafeLogger(const SafeLogger&) = delete;
    SafeLogger& operator=(const SafeLogger&) = delete;
    
    // Thread-safe logging
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        logFile << "[" << std::this_thread::get_id() << "] " 
                << message << std::endl;
    }
    
    // Thread-safe formatted logging
    void logError(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        logFile << "[ERROR][" << std::this_thread::get_id() << "] " 
                << message << std::endl;
    }
};

// ============================================================================
// GOOD EXAMPLE: Thread-Safe Counter
// ============================================================================
class SafeCounter {
private:
    mutable std::mutex mtx;
    int count;
    
public:
    SafeCounter() : count(0) {}
    
    // Thread-safe increment
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
    }
    
    // Thread-safe decrement
    void decrement() {
        std::lock_guard<std::mutex> lock(mtx);
        --count;
    }
    
    // Thread-safe read
    int getCount() const {
        std::lock_guard<std::mutex> lock(mtx);
        return count;  // Returns a copy, not a reference
    }
    
    // Thread-safe increment and return
    int incrementAndGet() {
        std::lock_guard<std::mutex> lock(mtx);
        return ++count;
    }
};

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

// Demo 1: Race condition with unsafe counter
void demo1_unsafe() {
    std::cout << "\n=== DEMO 1: Unsafe Counter (Race Condition) ===" << std::endl;
    
    UnsafeCounter counter;
    
    auto incrementTask = [&counter]() {
        for (int i = 0; i < 10000; ++i) {
            counter.increment();
        }
    };
    
    std::thread t1(incrementTask);
    std::thread t2(incrementTask);
    
    t1.join();
    t2.join();
    
    std::cout << "Expected count: 20000" << std::endl;
    std::cout << "Actual count: " << counter.getCount() << std::endl;
    std::cout << "Race condition likely caused incorrect result!" << std::endl;
}

// Demo 2: Data leak example
void demo2_leaky() {
    std::cout << "\n=== DEMO 2: Data Leak Danger ===" << std::endl;
    
    LeakyStack stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    
    // Get reference to internal data
    std::vector<int>& leakedData = stack.getData();
    
    // Now we can modify the internal data without holding the mutex!
    // This is dangerous in multithreaded context
    std::cout << "Modifying internal data without mutex protection..." << std::endl;
    leakedData.clear();  // DANGER: Modifying without lock!
    
    std::cout << "This demonstrates why leaking handles is dangerous!" << std::endl;
}

// Demo 3: Bad interface design
void demo3_bad_interface() {
    std::cout << "\n=== DEMO 3: Bad Interface Design ===" << std::endl;
    
    BadStack stack;
    stack.push(10);
    
    // This pattern has a race condition:
    // Thread A checks isEmpty() -> returns false
    // Thread B pops the item
    // Thread A calls top() -> CRASH! Stack is now empty
    
    std::cout << "Pattern: if (!stack.isEmpty()) { value = stack.top(); }" << std::endl;
    std::cout << "Problem: Another thread might pop between isEmpty() and top()" << std::endl;
    std::cout << "This is a Time-Of-Check-Time-Of-Use (TOCTOU) bug!" << std::endl;
}

// Demo 4: Safe stack usage
void demo4_safe_stack() {
    std::cout << "\n=== DEMO 4: Safe Stack (Proper Design) ===" << std::endl;
    
    SafeStack stack;
    
    // Producer thread
    auto producer = [&stack]() {
        for (int i = 0; i < 100; ++i) {
            stack.push(i);
        }
    };
    
    // Consumer thread
    auto consumer = [&stack]() {
        int sum = 0;
        int value;
        while (stack.tryPop(value)) {
            sum += value;
        }
        // Try a bit more to catch items still being produced
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while (stack.tryPop(value)) {
            sum += value;
        }
    };
    
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    std::cout << "Safe operations completed successfully!" << std::endl;
    std::cout << "Remaining items in stack: " << stack.size() << std::endl;
}

// Demo 5: Safe logger
void demo5_safe_logger() {
    std::cout << "\n=== DEMO 5: Safe Logger ===" << std::endl;
    
    SafeLogger logger("app.log");
    
    auto logTask = [&logger](int threadNum) {
        for (int i = 0; i < 50; ++i) {
            logger.log("Message " + std::to_string(i) + 
                      " from thread " + std::to_string(threadNum));
        }
    };
    
    std::thread t1(logTask, 1);
    std::thread t2(logTask, 2);
    std::thread t3(logTask, 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Check app.log - all entries are properly written!" << std::endl;
}

// Demo 6: Safe counter
void demo6_safe_counter() {
    std::cout << "\n=== DEMO 6: Safe Counter (No Race Condition) ===" << std::endl;
    
    SafeCounter counter;
    
    auto incrementTask = [&counter]() {
        for (int i = 0; i < 10000; ++i) {
            counter.increment();
        }
    };
    
    std::thread t1(incrementTask);
    std::thread t2(incrementTask);
    
    t1.join();
    t2.join();
    
    std::cout << "Expected count: 20000" << std::endl;
    std::cout << "Actual count: " << counter.getCount() << std::endl;
    std::cout << "Perfect! No race condition." << std::endl;
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
    std::cout << "=== DATA RACE PREVENTION DEMONSTRATIONS ===" << std::endl;
    
    demo1_unsafe();
    demo2_leaky();
    demo3_bad_interface();
    demo4_safe_stack();
    demo5_safe_logger();
    demo6_safe_counter();
    
    std::cout << "\n=== ALL DEMONSTRATIONS COMPLETE ===" << std::endl;
    
    return 0;
}