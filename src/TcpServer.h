#pragma once
#include "TcpConn.h"
#include <map>

namespace AsioNet
{
	class TcpServer {
	public:
		TcpServer() = delete;
		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		TcpServer(io_ctx& ctx);
		~TcpServer();
		void Serve(unsigned short port);
		ServerKey GetKey();
	protected:
		void doAccept();
	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
		std::map<NetKey, std::shared_ptr<TcpConn>> m_clients;
		std::mutex m_lock;
	};
}