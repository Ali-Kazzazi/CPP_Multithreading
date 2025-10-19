#include <iostream>
#include <thread>

// Function that will run in a separate thread
void func1(){
    // Loop 100 times, printing a message from thread T1
    for (int i = 0; i < 100; i++){
        std::cout << "--T1" << std::endl;
    }
}

int main()
{
    // Create and start a new thread that executes func1()
    // The thread begins running immediately
    std::thread t1(func1);
    
    // Use try-catch block to ensure thread is properly joined even if an exception occurs
    try{
        // Main thread loop: print 100 messages
        for (int i = 0; i < 100; i++){
            std::cout << "MAIN--" << std::endl;
        }
    }
    catch(...){
        // If any exception occurs in the main thread's work,
        // we must join the spawned thread before re-throwing
        // This prevents the thread from being orphaned
        t1.join();
        throw;  // Re-throw the exception after cleanup
    }
    
    // Wait for thread t1 to complete execution
    // This is critical: a thread must be joined or detached before destruction
    t1.join();
    
    return 0;
}