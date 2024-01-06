#include "TcpServer.h"
#include <iostream>

namespace AsioNet
{
	// #define ERR_CHECK if(ec){std::cout << ec.message() << std::endl;return;}

	TCPServer::TCPServer(io_context& ctx):
		m_acceptor(ctx)
	{

	}
	void TCPServer::Serve(unsigned short port)
	{
		ip::tcp::endpoint ep(ip::tcp::v4(), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();


		ip::tcp::socket cli(m_acceptor.get_executor());
		m_acceptor.async_accept(cli,boost::bind(&TCPServer::accept_handler,this,_1));

	}

	void TCPServer::accept_handler(const error_code& ec)
	{
		if (ec) { std::cout << ec.message() << std::endl; return; }

	}
}