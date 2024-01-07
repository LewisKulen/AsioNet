#pragma once
#include "TcpConn.h"

namespace AsioNet
{
	class TCPServer {
	public:
		TCPServer(io_ctx& context);
		~TCPServer();
		void Serve(unsigned short port);
	protected:
		void accept_handler(const NetErr& error,TcpSock cli);
	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
	};
}