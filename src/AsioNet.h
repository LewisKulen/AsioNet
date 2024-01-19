#pragma once

#include "AsioNetDef.h"
#include "event/IEventPoller.h"
#include "event/IEventHandler.h"
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

    class TcpNetMgr {
    public:
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/,IEventHandler*);
        ~TcpNetMgr();

        ServerKey Serve(unsigned short port/*,options*/);
        bool ServerSend(NetKey, const char* data, size_t trans);
        bool Broadcast(ServerKey, const char* data, size_t trans);

        void Connect(std::string ip, unsigned short port/*,options*/);       
        bool Send(NetKey, const char* data, size_t trans);
    private:
        io_ctx ctx;
        std::vector<std::thread> thPool;
        IEventPoller* ptr_poller;
        TcpConnMgr connMgr;
        TcpServerMgr serverMgr;
    };
}
