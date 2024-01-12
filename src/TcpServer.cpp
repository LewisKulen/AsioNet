#include "TcpServer.h"

namespace AsioNet
{
	// #define ERR_CHECK if(ec){std::cout << ec.message() << std::endl;return;}

	TcpServer::TcpServer(io_ctx& ctx):
		m_acceptor(ctx)
	{}

	TcpServer::~TcpServer()
	{
		NetErr err;
		m_acceptor.close(err);
	}

	void TcpServer::Serve(unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::tcp::v4(), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();

		DoAccept();
	}

	void TcpServer::DoAccept()
	{
		m_acceptor.async_accept([this](const NetErr& ec,TcpSock cli){
			if (ec) { return; }

			auto conn = std::make_shared<TcpConn>(std::move(cli));

			static NetErr err;
			conn->poller->PushAccept(conn->sock_.remote_endpoint(err));
			{
				std::lock_guard<std::mutex> guard(m_lock);
				this->m_clients[conn->GetKey()] = conn;
			}
			conn->StartRead();

			DoAccept();
		});
	}
	
}