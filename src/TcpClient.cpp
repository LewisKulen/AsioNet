#include "TcpClient.h"
#include <iostream>

namespace AsioNet
{
	TcpClient::TcpClient(io_context &ctx) : conn(ctx)
	{
	}

	void TcpClient::Connect(std::string addr, unsigned short port)
	{
		ip::tcp::endpoint ep(ip::address::from_string(addr.c_str()), port);
		conn.sock_.async_connect(ep, boost::bind(&TcpClient::connect_handler, this, boost::placeholders::_1));
	}

	void TcpClient::connect_handler(const error_code &ec)
	{
		if (ec)
		{
			std::cout << ec.message() << std::endl;
			return;
		}

		std::cout << "connect to : " << conn.sock_.remote_endpoint().address().to_string() << ":" << conn.sock_.remote_endpoint().port() << std::endl;
		conn.StartRead();
	}

	void TcpClient::Send(const char *data, unsigned short trans)
	{
		conn.Write(data, trans);
	}
	void TcpClient::Close()
	{
		conn.Close();
	}

}