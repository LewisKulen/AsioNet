#pragma once

#include "./KcpConn.h"
#include "../utils/utils.h"
#include <unordered_map>

namespace AsioNet
{
	class KcpConnMgr:public IKcpConnOwner {
	public:
		void DelConn(NetKey) override;
		void AddConn(std::shared_ptr<KcpConn>) override;
		
		void Disconnect(NetKey k); // 考虑断线重连
		std::shared_ptr<KcpConn> GetConn(NetKey);
		std::shared_ptr<KcpConn> GetConn(const UdpEndPoint&);
		void Broadcast(const char*,size_t);

		~KcpConnMgr();
	private:
		std::unordered_map<NetKey,std::shared_ptr<KcpConn>> m_conns;
		std::unordered_map<UdpEndPoint,NetKey> m_connHelper;
		std::mutex m_lock;
	};

	// ikcp_allocator:可以考虑接管内存管理
	class KcpServer : public std::enable_shared_from_this<KcpServer>
	{
	public:
		KcpServer() = delete;
		KcpServer(const KcpServer&) = delete;
		KcpServer(KcpServer&&) = delete;
		KcpServer& operator=(const KcpServer&) = delete;
		KcpServer& operator=(KcpServer&&) = delete;

		KcpServer(io_ctx& ctx, IEventPoller* p);

		~KcpServer();

		void Serve(const std::string& ip, int16_t port, uint32_t conv);
		
		bool Write(NetKey,const char* data, size_t trans);

		std::shared_ptr<KcpConn> GetConn(NetKey);

		void Broadcast(const char* data, size_t trans);
		
		void Disconnect(NetKey);

		ServerKey Key();
	protected:
		void readLoop();
		void err_handler();
	private:
		std::shared_ptr<UdpSock>	m_sock;		// underlying sock
		UdpEndPoint m_tempRecevier;	// 每次收到udp包时候的对端地址
		char m_kcpBuffer[AN_KCP_BUFFER_SIZE];
		ServerKey m_key;
		uint32_t m_conv;

		KcpConnMgr m_conns;
		IEventPoller* ptr_poller;
	};

}
