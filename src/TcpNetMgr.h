#pragma once

#include "AsioNetDef.h"
#include "event/IEventHandler.h"
#include "event/DefaultEventPoller.h"
#include "tcp/TcpServer.h"

#include <map>

namespace AsioNet
{

    /*
    NetMgr------>DefaultPoller--Handler
      |  \          ^
      |   \         |
    Server \    IEventPoller
      \___Conn_____/
    
    */
    class TcpServerMgr{
    public:
      std::shared_ptr<TcpServer> GetServer(ServerKey);
      void AddServer(std::shared_ptr<TcpServer>);
      ~TcpServerMgr();
    private:
      std::mutex m_lock;
      std::map<ServerKey,std::shared_ptr<TcpServer>> servers;
    };

    // 这是一个对客户端友好的NetMgr
    class TcpNetMgr {
    public:
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/,IEventHandler*);
        ~TcpNetMgr();

        // 直到把当前的请求全部处理完才结束，占用调用者的线程资源
        void Update();
        ServerKey Serve(unsigned short port/*,options*/);
        void Broadcast(ServerKey, const char* data, size_t trans);

        // retry：连接失败后的重试次数
        void Connect(std::string ip, unsigned short port,int retry = 1/*,options*/);       
        void Disconnect(NetKey);
        bool Send(NetKey, const char* data, size_t trans);

    private:
        io_ctx ctx;
        std::vector<std::thread> thPool;
        DefaultEventPoller m_poller;
        TcpConnMgr connMgr;
        TcpServerMgr serverMgr;
    };
}
