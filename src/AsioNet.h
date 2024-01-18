#pragma once
#include "AsioNetDef.h"
#include <thread>
#include "Event.h"

namespace AsioNet
{

    /*
    NetMgr
      |\____
    Server  |
      \___Conn--Poller--Handler
    
    */

    // 外部只能拿到一个NetKey,提供一个转化成addr的函数
    NetAddr NetKey2Addr(NetKey);

    struct TcpNetMgr {
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/,IEventHandler*);
        ~TcpNetMgr();

        void Serve(unsigned short port/*,options*/);
        void Connect(std::string ip, unsigned short port/*,options*/);        
        bool Send(NetKey, const char* data, size_t trans);

    private:
        io_ctx ctx;
        std::vector<std::thread> th_pool;
        IEventPoller* ptr_poller;
    };
}
