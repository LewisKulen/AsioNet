#include "TcpServer.h"
#include <iostream>

namespace AsioNet
{
	// #define ERR_CHECK if(ec){std::cout << ec.message() << std::endl;return;}

	TCPServer::TCPServer(io_ctx& ctx):
		m_acceptor(ctx)
	{}

	TCPServer::~TCPServer()
	{
		std::cout << "server destory" << std::endl;
		m_acceptor.close();
	}
	void acp_handler(const NetErr& ec,TcpSock cli)
	{
		if (ec) { std::cout << ec.message() << std::endl; return; }

	
		g_lock.lock();
		// problem 2:why local port always be 8888
		std::cout << "accept: " << cli.remote_endpoint().address().to_string() << ":" << cli.remote_endpoint().port() << std::endl;
		std::cout << "local_accept : " << cli.local_endpoint().address().to_string() << ":" << cli.local_endpoint().port() << std::endl;
		g_lock.unlock();
		

		auto conn = std::make_shared<TcpConn>(std::move(cli));
		conn->StartRead();
	}

	void TCPServer::Serve(unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::tcp::v4(), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();

		m_acceptor.async_accept(acp_handler);
		
		// problem 1
		// m_acceptor.async_accept(boost::bind(&TCPServer::accept_handler,this, boost::placeholders::_1, boost::placeholders::_2));

	}

	void TCPServer::accept_handler(const NetErr& ec,TcpSock cli)
	{
		 if (ec) { std::cout << ec.message() << std::endl; return; }
		 auto conn = std::make_shared<TcpConn>(std::move(cli));
		 conn->StartRead();
	}
	
}