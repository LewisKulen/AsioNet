#pragma once

#include "TcpConn.h"

namespace AsioNet
{
	class TcpServer: public std::enable_shared_from_this<TcpServer> {
	public:
		TcpServer() = delete;
		TcpServer(const TcpServer&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;

		TcpServer(io_ctx& ctx);
		~TcpServer();
		void Serve(unsigned short port);
	protected:
		void doAccept();
	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
	};
}