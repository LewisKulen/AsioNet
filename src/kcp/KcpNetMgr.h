#pragma once

// #include "../utils/AsioNetDef.h"
#include "../event/IEventPoller.h"
#include "KcpServer.h"

namespace AsioNet
{
    class KcpServerMgr{
    public:
      std::shared_ptr<KcpServer> GetServer(ServerKey);
      void AddServer(std::shared_ptr<KcpServer>);
      ~KcpServerMgr();
    private:
      std::mutex m_lock;
      std::unordered_map<ServerKey,std::shared_ptr<KcpServer>> servers;
    };

    // 注意：kcp这边断连判定需要心跳去解决，交给应用层去解决更为合理
    class KcpNetMgr {
    public:
        KcpNetMgr() = delete;
        KcpNetMgr(const KcpNetMgr&) = delete;
        KcpNetMgr(KcpNetMgr&&) = delete;
        KcpNetMgr& operator=(const KcpNetMgr&) = delete;
        KcpNetMgr& operator=(KcpNetMgr&&) = delete;

        KcpNetMgr(size_t th_num/*线程数量*/);
        ~KcpNetMgr();

        // ******************** 连接相关 ********************
        ServerKey Serve(IEventPoller* poller,const std::string& ip,uint16_t port, uint32_t conv);
        void Broadcast(ServerKey, const char* data, size_t trans);

        // 怎样算连接成功还有待商榷
        void Connect(IEventPoller* poller,const std::string& ip, uint16_t port, uint32_t conv);
        void Disconnect(NetKey);
        bool Send(NetKey, const char* data, size_t trans);
    private:
        io_ctx m_ctx;
        std::atomic<bool> m_isClose;
        std::vector<std::thread> thPool;
        KcpConnMgr m_connMgr;
        KcpServerMgr m_serverMgr;
    };

}
