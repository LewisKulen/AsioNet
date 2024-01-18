#include "AsioNet.h"
#include "test.h"

#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

// #include <boost/thread.hpp>

unsigned short port_ = 8888;

struct TestServerHandler:public AsioNet::IEventHandler{
    void AcceptHandler(AsioNet::NetKey){};
    void ConnectHandler(AsioNet::NetKey){};
    void DisconnectHandler(AsioNet::NetKey){};
    void RecvHandler(AsioNet::NetKey, const char *data, size_t trans){};
    void RecvHandler(AsioNet::NetKey, const std::string &){};
};

void TestServer()
{
    TestServerHandler h;
    AsioNet::TcpNetMgr net(2,&h);
    net.Serve(8888);
    std::cout << "finish" << std::endl;
}


struct TestClientHandler:public AsioNet::IEventHandler{
    void AcceptHandler(AsioNet::NetKey){};
    void ConnectHandler(AsioNet::NetKey){};
    void DisconnectHandler(AsioNet::NetKey){};
    void RecvHandler(AsioNet::NetKey, const char *data, size_t trans){};
    void RecvHandler(AsioNet::NetKey, const std::string &){};
};

void TestClient()
{
    TestClientHandler h;
    AsioNet::TcpNetMgr net(2,&h);
    net.Connect("127.0.0.1",8888);
    std::cout << "client finish" << std::endl;
}

int main()
{
    TestServer();
    // DoTestFunc();
    return 0;
}