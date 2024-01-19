#pragma once

#include "TcpConn.h"
#include <map>

namespace AsioNet
{
	class TcpConnMgr{
	public:
		std::shared_ptr<TcpConn> GetConn(NetKey);
		void DelConn(NetKey);
		void AddConn(std::shared_ptr<TcpConn>);
		void Broadcast(const char*,size_t trans);

		~TcpConnMgr();
	private:
		std::map<NetKey,std::shared_ptr<TcpConn>> conns;
		std::mutex m_lock;
	};

	class TcpServer: public std::enable_shared_from_this<TcpServer> {
	public:
		TcpServer() = delete;
		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		TcpServer(io_ctx& ctx,IEventPoller* p);
		~TcpServer();
		void Serve(unsigned short port);
		void Broadcast(const char*,size_t trans);
		std::shared_ptr<TcpConn> GetConn(NetKey k);
		ServerKey TcpServer::GetKey();
	protected:
		void doAccept();
	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
		
		TcpConnMgr connMgr;
		IEventPoller* ptr_poller;
		ServerKey m_key;
	};
}