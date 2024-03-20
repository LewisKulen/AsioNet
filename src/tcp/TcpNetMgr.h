#pragma once

// #include "../utils/AsioNetDef.h"
#include "../event/IEventPoller.h"
#include "../tcp/TcpServer.h"

namespace AsioNet
{

    /*
    TcpNetMgr         EventDriver---->Handler
      |     \             ^
      v      \            |
    TcpServer \           |
      \        |          |
       \       v          |
        -->TcpConn---->IEventPoller
    */
    class TcpNetMgr {
    public:
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr(TcpNetMgr&&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(TcpNetMgr&&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/);
        ~TcpNetMgr();

        // ******************** 连接相关 ********************
        ServerKey Serve(IEventPoller* poller,const std::string& ip,uint16_t port);
        void Broadcast(ServerKey, const char* data, size_t trans);

        void Connect(IEventPoller* poller,const std::string& ip, uint16_t port,int retry = 1/*连接失败后的重试次数*/);
        void Disconnect(NetKey);
        bool Send(NetKey, const char* data, size_t trans);
    private:
        io_ctx m_ctx;
        std::atomic<bool> m_isClose;
        std::vector<std::thread> thPool;
        TcpConnMgr m_connMgr;
        TcpServerMgr m_serverMgr;
    };

}
