#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>

using namespace std;

// ============================================================================
// LESSON: SHARED MUTEX (Reader-Writer Lock)
// ============================================================================
/*
WHAT IS A SHARED MUTEX?
- A std::shared_mutex allows two types of locks:
  1. SHARED LOCK (read lock): Multiple threads can hold this simultaneously
  2. EXCLUSIVE LOCK (write lock): Only ONE thread can hold this at a time

WHEN TO USE:
- When you have data that is READ often but WRITTEN rarely
- Example: Configuration settings, caches, lookup tables

PERFORMANCE BENEFIT:
- Multiple readers can work concurrently (no waiting)
- Writers have exclusive access (data consistency guaranteed)

KEY TYPES:
- std::shared_mutex: The mutex itself (C++17)
- std::shared_lock: For shared (read) access
- std::lock_guard/std::unique_lock: For exclusive (write) access
*/

// Global shared_mutex for demonstration
// In real code, this should be encapsulated in a class
shared_mutex sh_mutex;

// ============================================================================
// FUNCTION: print() - WRITER FUNCTION (Exclusive Lock)
// ============================================================================
// This function represents a WRITE operation
// It needs EXCLUSIVE access because it's modifying shared state (console output)
void print(int i) {
    // Using lock_guard with shared_mutex creates an EXCLUSIVE lock
    // Only ONE thread can execute this block at a time
    lock_guard<shared_mutex> lg(sh_mutex);
    
    cout << "print thread " << i << " with exclusive lock" << endl;
    
    // Sleep simulates expensive write operation
    // During this time, NO other thread (reader or writer) can proceed
    this_thread::sleep_for(2s);
    
    // Lock is automatically released when lg goes out of scope
}
/*
IMPORTANT NOTE ABOUT THIS IMPLEMENTATION:
This example uses lock_guard<shared_mutex> which creates an EXCLUSIVE lock,
but the function is named "print" and labeled as if it should be a write operation.
However, the lock type doesn't match a true read-only operation.

For a proper demonstration:
- Writers should use: lock_guard<shared_mutex> or unique_lock<shared_mutex>
- Readers should use: shared_lock<shared_mutex>
*/

// ============================================================================
// FUNCTION: read() - READER FUNCTION (Should Use Shared Lock!)
// ============================================================================
// This function represents a READ operation
// **BUG**: This is using lock_guard (exclusive) instead of shared_lock!
void read(int i) {
    // BUG: Using lock_guard creates an EXCLUSIVE lock
    // This defeats the purpose of shared_mutex!
    lock_guard<shared_mutex> sl(sh_mutex);
    
    // CORRECT VERSION (commented):
    // shared_lock<shared_mutex> sl(sh_mutex);  // Multiple readers can hold this
    
    cout << "Read thread " << i << " with shared lock" << endl;
    
    // Note: No sleep here, so read operations are fast
    
    // Lock is automatically released when sl goes out of scope
}
/*
PROBLEM WITH THIS CODE:
The read() function claims to use "shared lock" but actually uses lock_guard,
which creates an EXCLUSIVE lock. This means:
- Only ONE reader can execute at a time (defeats the purpose!)
- Readers will wait for each other unnecessarily

CORRECT IMPLEMENTATION:
void read(int i) {
    shared_lock<shared_mutex> sl(sh_mutex);  // SHARED lock
    cout << "Read thread " << i << " with shared lock" << endl;
}

With shared_lock, multiple readers can execute CONCURRENTLY!
*/

// ============================================================================
// CORRECTED VERSIONS (For Teaching Purposes)
// ============================================================================

// CORRECTED: Writer function with proper exclusive lock
void write_correct(int i) {
    // Exclusive lock: Only ONE writer at a time
    unique_lock<shared_mutex> lock(sh_mutex);
    
    cout << "WRITER thread " << i << " - exclusive access" << endl;
    
    // Simulate expensive write operation
    this_thread::sleep_for(chrono::milliseconds(500));
    
    cout << "WRITER thread " << i << " - finished writing" << endl;
}

// CORRECTED: Reader function with proper shared lock
void read_correct(int i) {
    // Shared lock: MULTIPLE readers can hold this simultaneously
    shared_lock<shared_mutex> lock(sh_mutex);
    
    cout << "READER thread " << i << " - shared access" << endl;
    
    // Simulate read operation (fast)
    this_thread::sleep_for(chrono::milliseconds(100));
    
    cout << "READER thread " << i << " - finished reading" << endl;
}

// ============================================================================
// DEMONSTRATION: Compare Original vs Corrected
// ============================================================================

void demo_original() {
    cout << "\n=== ORIGINAL VERSION (Bug: readers use exclusive lock) ===" << endl;
    cout << "Watch: Readers will wait for each other (BAD!)" << endl;
    cout << "Expected behavior: All operations serialize (slow)" << endl << endl;
    
    vector<thread> threads;
    
    // Create 5 reader threads
    for (int i = 0; i < 5; ++i)
        threads.push_back(thread{read, i});
    
    // Create 2 writer threads
    threads.push_back(thread{print, 5});
    threads.push_back(thread{print, 6});
    
    // Create 10 more reader threads
    for (int i = 0; i < 10; ++i)
        threads.push_back(thread{read, i+7});
    
    // Wait for all threads to complete
    for (auto& t : threads)
        t.join();
}

void demo_corrected() {
    cout << "\n=== CORRECTED VERSION (Readers use shared lock) ===" << endl;
    cout << "Watch: Multiple readers execute concurrently (GOOD!)" << endl;
    cout << "Expected behavior: Readers run in parallel, writers get exclusive access" << endl << endl;
    
    vector<thread> threads;
    
    // Create 5 reader threads
    for (int i = 0; i < 5; ++i)
        threads.push_back(thread{read_correct, i});
    
    // Create 2 writer threads
    threads.push_back(thread{write_correct, 5});
    threads.push_back(thread{write_correct, 6});
    
    // Create 10 more reader threads
    for (int i = 0; i < 10; ++i)
        threads.push_back(thread{read_correct, i+7});
    
    // Wait for all threads to complete
    for (auto& t : threads)
        t.join();
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main() {
    cout << "=== SHARED MUTEX (READER-WRITER LOCK) LESSON ===" << endl;
    cout << "\nThis lesson demonstrates the difference between:" << endl;
    cout << "1. EXCLUSIVE LOCK (lock_guard/unique_lock): ONE thread at a time" << endl;
    cout << "2. SHARED LOCK (shared_lock): MULTIPLE threads simultaneously" << endl;
    
    // Run original buggy version
    demo_original();
    
    cout << "\n" << string(70, '=') << "\n" << endl;
    
    // Run corrected version
    demo_corrected();
    
    cout << "\n=== LESSON COMPLETE ===" << endl;
    cout << "\nKEY TAKEAWAYS:" << endl;
    cout << "1. Use shared_lock<shared_mutex> for READ operations" << endl;
    cout << "2. Use lock_guard/unique_lock<shared_mutex> for WRITE operations" << endl;
    cout << "3. Multiple readers can execute concurrently" << endl;
    cout << "4. Writers have exclusive access (block all readers and writers)" << endl;
    cout << "5. Use shared_mutex when reads are frequent and writes are rare" << endl;
    
    return 0;
}

// ============================================================================
// PERFORMANCE COMPARISON
// ============================================================================
/*
SCENARIO: 100 reads, 2 writes

WITH EXCLUSIVE LOCKS (wrong way):
- All 102 operations execute sequentially
- Total time: 102 × operation_time
- NO parallelism

WITH SHARED LOCKS (correct way):
- 100 reads can execute concurrently
- 2 writes execute exclusively (serialize)
- Total time: max(read_time) + 2 × write_time
- MASSIVE parallelism for reads

SPEEDUP: Can be 10x-100x faster for read-heavy workloads!
*/

// ============================================================================
// COMMON USE CASES
// ============================================================================
/*
1. CONFIGURATION CACHE
   - Many threads read config (shared lock)
   - Rare updates reload config (exclusive lock)

2. DATABASE QUERY CACHE
   - Many threads read cache entries (shared lock)
   - Cache invalidation/update uses exclusive lock

3. REFERENCE COUNTING
   - Many threads read reference count (shared lock)
   - Increment/decrement uses exclusive lock

4. LOOKUP TABLES
   - Many threads lookup values (shared lock)
   - Table updates use exclusive lock

5. OBSERVER PATTERN
   - Many threads read observer list (shared lock)
   - Add/remove observers uses exclusive lock
*/

// ============================================================================
// BEST PRACTICES
// ============================================================================
/*
DO:
✅ Use shared_lock for true read-only operations
✅ Use exclusive locks (lock_guard/unique_lock) for writes
✅ Keep critical sections as small as possible
✅ Profile before using shared_mutex (overhead for simple cases)
✅ Encapsulate shared_mutex with the data it protects

DON'T:
❌ Use shared_mutex for write-heavy workloads (overhead not worth it)
❌ Use exclusive locks for reads (defeats the purpose!)
❌ Mix shared_mutex with regular mutex in same critical section
❌ Hold shared locks while calling external functions
❌ Assume shared_mutex is always faster (profile first!)

WHEN NOT TO USE:
- If writes are as common as reads → use regular mutex
- If critical sections are very short → overhead not worth it
- If you have simple data → consider atomic types instead
*/