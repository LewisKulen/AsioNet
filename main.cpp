#include "TcpServer.h"
#include "TcpClient.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

// #include <boost/thread.hpp>

unsigned short port_ = 8888;

std::mutex g_lock;

void TestServer()
{
    AsioNet::io_ctx ctx;
    std::vector<std::thread> thread_pool;
    std::atomic<int> tot = 0;
    int th_num = 4;
    for (int i = 0;i < th_num; ++i)
    {
        thread_pool.push_back(
            std::thread([&](){
                tot++;
            while(true)
            {
                ctx.run();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        })
        );
    }

    AsioNet::TcpServer s(ctx);
    s.Serve(port_);

    for(auto& t : thread_pool)
    {
        t.join();
    }
    std::cout << "finish" << std::endl;
}

void TestClient()
{
    AsioNet::io_ctx ctx;
    std::vector<std::thread> thread_pool;
    std::atomic<int> tot = 0;
    int th_num = 4;
    for (int i = 0;i < th_num; ++i)
    {
        thread_pool.push_back(
            std::thread([&](){
                tot++;
            while(true)
            {
                ctx.run();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        })
        );
    }

    AsioNet::TcpClient c(ctx);
    c.Connect("127.0.0.1",8888);

    int pos = 0;
    int loop = 3;
    for(int i = 1;;++i)
    {
        auto data = std::to_string(i);
        c.Send(data.c_str(),data.length());
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    // c.Close();

    std::cout << "client finish" << std::endl;

    for(auto& t : thread_pool)
    {
        t.join();
    }
    std::cout << "finish" << std::endl;
}


int main()
{
    TestServer();
    // Test();
    // TestClient();
    return 0;
}