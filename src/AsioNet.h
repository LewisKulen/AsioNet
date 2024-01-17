#pragma once
#include "AsioNetDef.h"
#include "Event.h"

#include <thread>

namespace AsioNet
{
    struct TcpNetMgr {
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/);
        ~TcpNetMgr();

        void Serve(unsigned short port/*,options*/);
        void Connect(std::string ip, unsigned short port/*,options*/);        
        bool Send(NetKey, const char* data, size_t trans);

        void SetHandler(IEventHandler*);

        // update
        // void OnRecv(NetKey, const char* data, size_t trans);
        // void OnRecv(NetKey, const std::string& data);    // for protobuf

        // void OnConnect(NetKey, const std::string& ip, unsigned short port);
        // void OnAccept(NetKey, const std::string& ip, unsigned short port);
        // void OnDisconnect(NetKey);
    private:
        IEventHandler* handler;
        io_ctx ctx;
        std::vector<std::thread> th_pool;
    };

    // 外部只能拿到一个NetKey,提供一个转化成addr的函数
    NetAddr NetKey2Addr(const NetKey&);
}
