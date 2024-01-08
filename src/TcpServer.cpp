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
		if(m_acceptor.is_open())
		{
			m_acceptor.close();
		}
	}

	void TCPServer::Serve(unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::tcp::v4(), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();

		DoAccept();
	}

	void TCPServer::DoAccept()
	{
		m_acceptor.async_accept([&](const NetErr& ec,TcpSock cli){
			if (ec) { std::cout << ec.message() << std::endl; return; }

			g_lock.lock();
			std::cout << "accept: " << cli.remote_endpoint().address().to_string() << ":" << cli.remote_endpoint().port() << std::endl;
			g_lock.unlock();
		
			auto conn = std::make_shared<TcpConn>(std::move(cli));
			conn->StartRead();
			DoAccept();
		});
	}

	
}