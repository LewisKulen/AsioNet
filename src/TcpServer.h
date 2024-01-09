#pragma once
#include "TcpConn.h"

namespace AsioNet
{
	class TCPServer {
	public:
		TCPServer(io_ctx& ctx);
		~TCPServer();
		void Serve(unsigned short port);
	protected:
		void DoAccept();

	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
	};
}