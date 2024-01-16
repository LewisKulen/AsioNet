#pragma once
#include "AsioNetDef.h"
#include "TcpServer.h"
#include "TcpClient.h"

namespace AsioNet
{
    //class NetMgr
    //{
    //public:
    //    // 解耦的话会有一层接口开销，就不用，网络库快要紧
    //    // std::shared_ptr<TcpServer> NewTcpServer();
    //    // std::shared_ptr<TcpClient> NewTcpClient();
    //private:
    //    // ServerMgr
    //    // ClientMgr
    //    io_ctx ctx;
    //};

    // 思考应该提供什么样的接口给使用者
    struct TcpNetMgr {
        TcpNetMgr(size_t th_num/*线程数量*/);
        void Serve(unsigned short port/*,options*/);
        void Connect(std::string ip, unsigned short port/*,options*/);

        bool Send(NetKey, const char* data, size_t trans);

        // update
        void OnRecv(NetKey, const char* data, size_t trans);
        void OnRecv(NetKey, const std::string& data);    // for protobuf

        void OnConnect(NetKey, const std::string& ip, unsigned short port);
        void OnAccept(NetKey, const std::string& ip, unsigned short port);
        void OnDisconnect(NetKey);
    private:
        io_ctx ctx;
    };
}
