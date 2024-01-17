#include "AsioNet.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

// #include <boost/thread.hpp>

unsigned short port_ = 8888;

void log(const char* msg){
    static std::mutex lock;
    std::lock_guard<std::mutex> guard(lock);
    std::cout << std::string(msg).c_str() << std::endl;
}


void TestServer()
{
    AsioNet::TcpNetMgr net(2);
    net.Serve(8888);
    std::cout << "finish" << std::endl;
}

void TestClient()
{
    AsioNet::TcpNetMgr net(2);
    net.Connect("127.0.0.1",8888);


    // int pos = 0;
    // int loop = 3;
    // for(int i = 1;;++i)
    // {
    //     auto data = std::to_string(i);
    //     c.Send(data.c_str(),data.length());
    //     std::this_thread::sleep_for(std::chrono::seconds(3));
    // }

    std::cout << "client finish" << std::endl;
}

void work(int a,int b)
{
    a++;
    b++;
    a + b;
}

auto lambda = [](){
    work(1,2);
};

struct ITest{
    virtual void  operator()() = 0;
    // virtual void test() = 0;
};

struct Test:public ITest{
    void operator()() override{
        work(1,2);
    }
};

template <typename Duration>
Duration TestFunc(size_t testNum,int a,int b) 
{
    auto t1 = std::chrono::system_clock::now();
    for(size_t i = 0;i < testNum;++i)
    {
        work(a,b);
    }
    auto t2 = std::chrono::system_clock::now();
    return std::chrono::duration_cast<Duration>(t2-t1);
}

// Duration should be : std::chrono::duration<>
template <typename T,typename Duration>
Duration TestFunc(T func,size_t testNum) 
{
    auto t1 = std::chrono::system_clock::now();
    for(size_t i = 0;i < testNum;++i)
    {
        func();
    }
    auto t2 = std::chrono::system_clock::now();
    return std::chrono::duration_cast<Duration>(t2-t1);
}

void DoTestFunc()
{
    /*
    普通函数：仿函数：虚函数调用，测试结果性能差距：平均耗时在19:24:24,20~25%的性能损耗
    */

    int iTestNum = 100'0000;
    
    auto c1 = TestFunc<std::chrono::microseconds>(iTestNum,1,2);
    Test t;
    auto c2 = TestFunc<Test,std::chrono::microseconds>(t,iTestNum);
    auto c3 = TestFunc<decltype(lambda),std::chrono::microseconds>(lambda,iTestNum);

    std::cout << "func:" << c1.count() << std::endl;
    std::cout << "functor:" << c2.count() << std::endl;
    std::cout << "lambda:" << c3.count() << std::endl;

}

int main()
{
    // TestServer();
    DoTestFunc();
    return 0;
}