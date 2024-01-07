#include "TcpServer.h"
#include "TcpClient.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

unsigned short port_ = 8888;

std::mutex g_lock;

void StartServer()
{
    AsioNet::io_ctx ctx;
    AsioNet::TCPServer s(ctx);
    s.Serve(port_);
    ctx.run();
    std::cout << "server quit" << std::endl;
}

void StartClient()
{
    AsioNet::io_ctx ctx;
    AsioNet::TcpClient c(ctx);
    c.Connect("127.0.0.1",8888);
    AsioNet::TcpClient c1(ctx);
    c1.Connect("127.0.0.1", 8888);

    /*AsioNet::TcpClient c2(ctx);
    c2.Send("lala", 4);*/

    auto t1 = std::thread([&ctx](){
        ctx.run();
    });

    int tot = 1024*1024*8;
    std::string s;
    int i = 1;
    while(s.length() < tot) {
        auto temp = std::to_string(i++);
        for (auto ch : temp)
        {
            s.push_back(ch);
        }
    }
    
    int pos = 0;
    int loop = 3;
    while(loop--)
    {
        c.Send(s.c_str(),100);
        c1.Send(s.c_str(), 6);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    c.Close();
    t1.join();
    std::cout << "client finish" << std::endl;
}

int main()
{
    auto t1 = std::thread(StartServer);
    auto t2 = std::thread(StartClient);
    t1.join();
    t2.join();
    return 0;
}