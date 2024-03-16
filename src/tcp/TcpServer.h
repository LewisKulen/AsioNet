#pragma once

#include "TcpConn.h"
#include <map>

namespace AsioNet
{
	class TcpConnMgr:public ITcpConnOwner {
	public:
		void DelConn(NetKey) override;
		void AddConn(std::shared_ptr<TcpConn>) override;
		void Disconnect(NetKey k);

		std::shared_ptr<TcpConn> GetConn(NetKey);
		void Broadcast(const char*,size_t trans);

		~TcpConnMgr();
	private:
		std::map<NetKey,std::shared_ptr<TcpConn>> m_conns;
		std::mutex m_lock;
	};

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
}