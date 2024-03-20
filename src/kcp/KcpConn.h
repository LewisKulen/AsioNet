#pragma once

#include <ikcp.h>

#include "../utils/AsioNetDef.h"
#include "../utils/BlockBuffer.h"
#include "../event/IEventPoller.h"

// 参考资料
// doc:https://github.com/libinzhangyuan/asio_kcp
// doc:https://pkg.go.dev/github.com/xtaci/kcp-go
// doc:https://luyuhuang.tech/2020/12/09/kcp.html

namespace AsioNet
{
	using UdpEndPoint = asio::ip::udp::endpoint;
	using UdpSock = asio::ip::udp::socket;
	
	const size_t AN_KCP_BUFFER_SIZE = 2048;

	struct IKcpConnOwner;
	enum class KcpConnMode
	{
		KCM_SERVER	= 1,
		KCM_CLIENT	= 2,
	};

	const uint32_t IKCP_OVERHEAD = 24;
	const uint32_t IKCP_MTU = 1400;	// default
	// ikcp_allocator:可以考虑接管内存管理
	// 请使用shared_ptr管理对象
	class KcpConn : public std::enable_shared_from_this<KcpConn>
	{
	public:
		KcpConn() = delete;
		KcpConn(const KcpConn&) = delete;
		KcpConn(KcpConn&&) = delete;
		KcpConn& operator=(const KcpConn&) = delete;
		KcpConn& operator=(KcpConn&&) = delete;
		
		// for server
		KcpConn(std::shared_ptr<UdpSock>,const UdpEndPoint&,IEventPoller* p,uint32_t conv);
		
		// for client
		KcpConn(io_ctx& ,IEventPoller*);

		// 目前其实是同步连接
		void Connect(const std::string& ip,uint16_t port,uint32_t conv);

		~KcpConn();

		void SetOwner(IKcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		void Close();

		NetKey Key();
		
		UdpEndPoint Remote();

		void KcpInput(const char* data,size_t trans);
		
		void KcpUpdate();

	protected:
		// 服务器创建的conn使用的output
	    static int kcpOutPutFunc(const char *buf, int len,ikcpcb *kcp, void *user);
		// connect用的output
		static int kcpOutPutFunc1(const char *buf, int len,ikcpcb *kcp, void *user);

		void readLoop();

		void init();

		void initKcp();

		void err_handler();
	private:
        // kcpsvr中，多个kcp依赖在一个udpsock上，所以这里使用了shared_ptr
		std::shared_ptr<UdpSock> m_sock;

		// kcp相关
		uint32_t m_conv;
        ikcpcb *m_kcp = nullptr;
		asio::high_resolution_timer m_updater;
		std::mutex m_kcpLock;

		// 对端addr
		UdpEndPoint m_sender;
		
        // 用于接受kcp协议的buffer，kcp协议经过分片处理，不需要很大
		char m_kcpBuffer[AN_KCP_BUFFER_SIZE];
		char m_readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;
		KcpConnMode m_mode;
		
		IEventPoller* ptr_poller;
		IKcpConnOwner* ptr_owner;
	};

	struct IKcpConnOwner {
		virtual void AddConn(std::shared_ptr<KcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~IKcpConnOwner() {}
	};

	class KcpConnMgr:public IKcpConnOwner {
	public:
		void DelConn(NetKey) override;
		void AddConn(std::shared_ptr<KcpConn>) override;
		
		void Disconnect(NetKey k);
		std::shared_ptr<KcpConn> GetConn(NetKey);
		std::shared_ptr<KcpConn> GetConn(const UdpEndPoint&);
		void Broadcast(const char*,size_t);

		~KcpConnMgr() override;
	private:
		std::unordered_map<NetKey,std::shared_ptr<KcpConn>> m_conns;
		std::unordered_map<UdpEndPoint,NetKey> m_connHelper;
		std::mutex m_lock;
	};
}
