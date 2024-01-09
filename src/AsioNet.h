#pragma once
#include "AsioNetDef.h"
#include "TcpServer.h"
#include "TcpClient.h"

namespace AsioNet
{
    class NetMgr
    {
    public:
        // 解耦的话会有一层接口开销，就不用，网络库快要紧
        // std::shared_ptr<TcpServer> NewTcpServer();
        // std::shared_ptr<TcpClient> NewTcpClient();
    private:
        // ServerMgr
        // ClientMgr
        io_ctx ctx;
    };
}