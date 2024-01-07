#include "TcpServer.h"
#include "TcpClient.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

unsigned short port_ = 8888;

void StartServer()
{
    AsioNet::io_context ctx;
    AsioNet::TCPServer s(ctx);
    s.Serve(port_);
    ctx.run();
    std::cout << "server quit" << std::endl;
}

void StartClient()
{
    AsioNet::io_context ctx;
    AsioNet::TcpClient c(ctx);
    c.Connect("127.0.0.1",8888);
    
    std::thread([&ctx](){
        ctx.run();
    });

    int tot = 1024*1024*8;
    std::string s;
    int i = 1;
    while(s.length() < tot) {
        s = s + std::to_string(i++);
    }
    
    int pos = 0;
    int loop = 1;
    while(true)
    {
        if (pos >= s.length()){
            break;
        }
        c.Send(s.c_str() + pos,loop);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    c.Close();
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