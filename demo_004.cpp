#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

typedef std::mutex mutex_t;
mutex_t mtx;

void dispMessage1(std::string s)
{
    mtx.lock();
    std::cout << s << std::endl;
    mtx.unlock();
}

void dispMessage2(std::string s)
{

    // RAII
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << s << std::endl;
}

// binding mutex to resource
class Logger
{
    std::mutex mtx;
    std::ofstream f;

public:
    Logger()
    {
        f.open("app.log");
    }

    virtual ~Logger()
    {
        if(f.is_open()) f.close();  
    }

    void log(std::string s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        f << s << std::endl;
    }
};

void func1()
{
    for (int i = 0; i < 100; i++)
    {
        std::cout << "T1 ---\n";
    }
}

void func2()
{
    for (int i = 0; i < 100; i++)
    {
        dispMessage1("T2 ---");
    }
}

void func3()
{
    for (int i = 0; i < 100; i++)
    {
        dispMessage2("T2 ---");
    }
}

void func4(Logger &logger)
{
    for (int i = 0; i < 100; i++)
    {
        logger.log("T1 ---");
    }
}

void demo1()
{
    std::thread t(func1);

    try
    {
        for (int i = 0; i > -100; --i)
        {
            std::cout << "--- main\n";
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
    std::thread t(func2);

    try
    {
        for (int i = 0; i > -100; --i)
        {
            dispMessage1("--- main");
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
}

void demo3()
{
    std::thread t(func3);

    try
    {
        for (int i = 0; i > -100; --i)
        {
            dispMessage2("--- main");
        }
    }
    catch (...)
    {
        t.join();
        throw;
    }

    t.join();
}

void demo4(){
    Logger logger_;

    std::thread t(func4, std::ref(logger_));

    try{
        for (int i = 0; i > -100; --i)
        {
            logger_.log("--- main");
        }
    }
    catch(...){
        t.join();
        throw;
    }

    t.join();

}

int main()
{

    demo1();
    demo2();
    demo3();
    demo4();

    return 0;
}