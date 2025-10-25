#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>


class Logger1
{
    std::mutex mtx;
    std::mutex mtx2;
    std::ofstream f;     

public:
    Logger1()
    {
        f.open("app.log");
    }

    virtual ~Logger1()
    {
        if(f.is_open()) f.close();  
    }

    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::lock_guard<std::mutex> lock2(mtx2);
        std::cout << s << std::endl;
    }

    void log2(std::string s)
    {
        std::lock_guard<std::mutex> lock2(mtx2);
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << s << std::endl;
    }


};

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
        if(f.is_open()) f.close();  
    }

    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::lock_guard<std::mutex> lock2(mtx2);
        std::cout << s << std::endl;
    }

    void log2(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::lock_guard<std::mutex> lock2(mtx2);
        std::cout << s << std::endl;
    }


};

void func1(Logger1 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---");  // Thread-safe logging
    }
}

void func2(Logger2 &logger)
{
    for (int i = 0; i < 1000; i++)
    {
        logger.log("T1 ---");  // Thread-safe logging
    }
}

void demo1()
{
    Logger1 logger;
    std::thread t(func1, std::ref(logger));

    try
    {
        
        for (int i = 0; i > -1000; --i)
        {
            logger.log2("--- main");  
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
}

void demo2()
{
    Logger2 logger;
    std::thread t(func2, std::ref(logger));

    try
    {
        
        for (int i = 0; i > -1000; --i)
        {
            logger.log2("--- main");  
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
}

int main()
{
    // demo1();
    demo2();


    return 0;
}