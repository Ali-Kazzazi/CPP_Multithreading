#include <iostream>
#include <thread>
#include <string>

// Functor class with no parameters
// Demonstrates basic functor usage with threads
class MyFunctor1{
public:
    void operator()(){ std::cout << "--T" << std::endl; }
};

// Functor class that accepts a reference parameter
// Used to demonstrate passing arguments by reference to threads
class MyFunctor2{
public:
    void operator()(std::string& txt){ std::cout << "--" + txt << std::endl; }
};

// Functor class that accepts a value parameter
// Used to demonstrate passing arguments by value/move to threads
class MyFunctor3{
public:
    void operator()(std::string txt){ std::cout << "--" + txt << std::endl; }
};

// DEMO 1: Basic functor with thread
// Shows how to create a functor object and pass it to a thread
void demo1()
{
    MyFunctor1 fuctr;
    std::thread t(fuctr);  // Pass functor to thread constructor

    try{
        std::cout << "MAIN--" << std::endl;
    }
    catch(...){
        t.join();  // Ensure thread is joined on exception
        throw;
    }
    
    t.join();  // Wait for thread to complete
}

// DEMO 2: Creating functor on-the-fly
// Demonstrates creating a temporary functor object inline
// Extra parentheses prevent "most vexing parse" ambiguity
void demo2()
{
    std::thread t((MyFunctor1()));  // Create functor object inline

    try{
        std::cout << "MAIN--" << std::endl;
    }
    catch(...){
        t.join();
        throw;
    }
    
    t.join();
}

// DEMO 3: Passing arguments by reference
// Uses std::ref() to pass a reference to the thread
// Both main thread and spawned thread can access the same variable
void demo3()
{
    std::string mytext = "message to T"; 
    // std::ref() wraps the reference so it can be passed to the thread
    std::thread t((MyFunctor2()), std::ref(mytext));

    try{
        // mytext is still accessible and valid in main thread
        std::cout << "MAIN--" << mytext << std::endl;
    }
    catch(...){
        t.join();
        throw;
    }
    
    t.join();
}

// DEMO 4: Passing arguments using move semantics
// Transfers ownership of mytext to the thread
// WARNING: mytext becomes invalid in main thread after move
void demo4()
{
    std::string mytext = "message to T"; 
    // std::move() transfers ownership of mytext to the thread
    std::thread t((MyFunctor3()), std::move(mytext));

    // DANGER: mytext is now in a moved-from state (empty string)
    // Accessing it prints an empty string, not a segfault
    // The comment about segfault is misleading - it's actually safe but empty
    try{
        std::cout << "MAIN--" << mytext << std::endl;  // Prints empty string
    }
    catch(...){
        t.join();
        throw;
    }
    
    t.join();
}

// DEMO 5: Moving thread objects
// Demonstrates that threads cannot be copied, only moved
// Transfers ownership of thread from t to t2
void demo5()
{
    std::string mytext = "message to T"; 
    std::thread t((MyFunctor3()), std::move(mytext));

    // Transfer thread ownership from t to t2
    // After this, t no longer manages a thread
    std::thread t2 = std::move(t);

    // Note: std::thread is move-only, not copyable
    // t is now in a "non-joinable" state
    try{
        std::cout << "MAIN--" << std::endl;
    }
    catch(...){
        t2.join();  // Must join t2, not t
        throw;
    }
    
    t2.join();
}

// DEMO 6: Thread IDs
// Shows how to get and track thread identifiers
// Demonstrates that moving a thread transfers its ID
void demo6()
{
    // Get the ID of the current (main) thread
    std::cout << "main thread ID:" << std::this_thread::get_id() << std::endl;

    std::thread t((MyFunctor1()));

    // Print the ID of thread t
    std::cout << "t ID:" << t.get_id() << std::endl;
    
    // Move thread t to t2
    std::thread t2 = std::move(t);

    // After move, t no longer has a valid thread ID (prints "thread::id of a non-executing thread")
    std::cout << "t ID:" << t.get_id() << std::endl;
    // t2 now has the original thread ID that t had
    std::cout << "t2 ID:" << t2.get_id() << std::endl;

    try{
        std::cout << "MAIN--" << std::endl;
    }
    catch(...){
        t2.join();
        throw;
    }
    
    t2.join();
}

int main()
{
    // Query the number of concurrent threads supported by the hardware
    // Useful for avoiding over-subscription (creating too many threads)
    std::cout << "get maximum number of threads: " << std::thread::hardware_concurrency() << std::endl;

    // Run all demonstrations sequentially
    std::cout << "DEMO 1--" << std::endl;
    demo1();
    
    std::cout << "DEMO 2--" << std::endl;
    demo2();
    
    std::cout << "DEMO 3--" << std::endl;
    demo3();
    
    std::cout << "DEMO 4--" << std::endl;
    demo4();
    
    std::cout << "DEMO 5--" << std::endl;
    demo5();
    
    std::cout << "DEMO 6--" << std::endl;
    demo6();
    
    return 0;
}