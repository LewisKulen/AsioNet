#pragma once
#include "TcpConn.h"
#include <map>

namespace AsioNet
{
	class TcpServer {
	public:
		TcpServer(io_ctx& ctx);
		~TcpServer();
		void Serve(unsigned short port);
	protected:
		void DoAccept();
	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
		std::map<NetKey, std::shared_ptr<TcpConn>> m_clients;
		std::mutex m_lock;
		std::shared_ptr<TcpConn> comming;
	};
}