#pragma once
#include "TcpConn.h"

namespace AsioNet
{
	class TCPServer {
	public:
		TCPServer(io_context& context);
		void Serve(unsigned short port);
	protected:
		void accept_handler(const error_code& error,ip::tcp::socket& cli);
	private:
		ip::tcp::acceptor m_acceptor;
	};
}