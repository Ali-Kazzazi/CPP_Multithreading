#include <iostream>
#include <string>
#include <thread>

// Class demonstrating various ways to create threads
// Contains regular methods, overloaded operators, and different signatures
class MYCLASS{
public:
    // Regular member function with parameters
    void func1(int i, std::string s){ 
        std::cout << i << " " << s << std::endl;
    }
    
    // Another member function with different signature
    long func2(double){
        return 0;
    }
    
    // Overloaded function call operator with parameter
    // Makes objects of this class callable like: obj(5)
    int operator()(int x){
        return 0;
    }
    
    // Overloaded function call operator without parameters
    // Makes objects of this class callable like: obj()
    void operator()(){}
};


int main(){
    MYCLASS cl;
    
    // T1: Pass object by value (copy) with one argument
    // The thread gets a COPY of cl, and calls operator()(1)
    // Since operator()(int) exists, it will be invoked with argument 1
    std::thread t1(cl, 1);
    
    // T2: Pass object by reference with one argument
    // The thread uses a REFERENCE to cl, and calls operator()(2)
    // std::ref() wraps the reference for safe passing to the thread
    std::thread t2(std::ref(cl), 2);
    
    // T3: Pass object using move semantics with one argument
    // Transfers ownership of cl to the thread, calling operator()(3)
    // After this, cl is in a moved-from state (still valid but unspecified)
    std::thread t3(std::move(cl), 3);
    
    // T4: Create temporary object inline and pass arguments
    // Creates a new MYCLASS object and calls operator()(4, "aaaa")
    // However, operator() doesn't match this signature, so this invokes
    // the implicit conversion or may not compile depending on the operator
    std::thread t4(MYCLASS(), 4, "aaaa");  
    
    // T5: Create temporary object with no additional arguments
    // Creates a new MYCLASS object and calls operator()()
    // Uses the parameterless operator()
    std::thread t5(MYCLASS()); 
    
    // T6: Lambda function as thread target
    // Lambda captures nothing [] and takes a string parameter
    // Demonstrates inline function definition for threads
    std::thread t6([](std::string s){
        std::cout << "output: " << s+s << std::endl; 
        return s+s;  // Return value is ignored by thread
    }, "CO");
    t6.join();  // Wait for lambda thread to complete
    
    // T7: Call member function on a COPY of the object
    // &MYCLASS::func1 is a pointer to member function
    // First argument (cl) is copied into the thread
    // Calls func1(7, "CO") on the copy
    std::thread t7(&MYCLASS::func1, cl, 7, "CO");
    t7.join();
    
    // T8: Call member function on a POINTER to the object
    // &cl passes a pointer to the original object
    // The thread calls func1(8, "CO") on the original object via pointer
    // CAUTION: The object must remain valid for the thread's lifetime
    std::thread t8(&MYCLASS::func1, &cl, 8, "CO");
    t8.join();
    
    // WARNING: t1, t2, t3, t4, and t5 are never joined!
    // This is a bug - the program will terminate when these threads
    // go out of scope without being joined, causing std::terminate()
    
    return 0;
}