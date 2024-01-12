#include "TcpClient.h"

namespace AsioNet
{
	TcpClient::TcpClient(io_ctx&ctx):conn(std::make_shared<TcpConn>(ctx))
	{}

	void TcpClient::Connect(std::string addr, unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::address::from_string(addr.c_str()), port);
		conn->sock_.async_connect(ep, boost::bind(&TcpClient::connect_handler, this, boost::placeholders::_1));
	}

	void TcpClient::connect_handler(const NetErr &ec)
	{
		if (ec)
		{
			conn->Close();
			return;
		}
		static NetErr err;
		conn->poller->PushConnect(conn->sock_.remote_endpoint(err));
		conn->StartRead();
	}

	// can only use when connect succ
	void TcpClient::Send(const char *data, size_t trans)
	{
		conn->Write(data, trans);
	}
	void TcpClient::Close()
	{
		conn->Close();
	}

}