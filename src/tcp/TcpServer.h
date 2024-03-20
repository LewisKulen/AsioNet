#pragma once

#include "TcpConn.h"
#include <map>

namespace AsioNet
{
	class TcpServer: public std::enable_shared_from_this<TcpServer> {
	public:
		TcpServer() = delete;
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(io_ctx& ctx,IEventPoller* p);
		
		~TcpServer();

		void Serve(const std::string& ip, uint16_t port);

		void Disconnect(NetKey);

		void Broadcast(const char*,size_t trans);

		std::shared_ptr<TcpConn> GetConn(NetKey k);

		ServerKey Key();

	protected:
		void doAccept();

	private:
		asio::ip::tcp::acceptor m_acceptor;
		
		TcpConnMgr connMgr;
		IEventPoller* ptr_poller;
		ServerKey m_key;
	};

	// 自己用的一个简易Server管理器
    class TcpServerMgr{
    public:
      std::shared_ptr<TcpServer> GetServer(ServerKey);
      void AddServer(std::shared_ptr<TcpServer>);
      ~TcpServerMgr();
    private:
      std::mutex m_lock;
      std::unordered_map<ServerKey,std::shared_ptr<TcpServer>> servers;
    };
}