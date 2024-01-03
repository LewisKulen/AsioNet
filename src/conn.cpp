#include "conn.h"
#include <iostream>

namespace AsioNet {

	TcpConn::TcpConn(io_service& s) :sock(s)
	{

	}

	bool TcpConn::Write(const char* acData, int iSize)
	{
		// 观buffer代码，只是简单的指针拷贝
		// 需要找到async_send内部在同个线程内，
		// 对这块数据进行拷贝放到系统缓冲区里面才安心
		sock.async_send(buffer(acData, iSize), boost::bind(&TcpConn::send_handler, this, _1, _2));
		return true;
	}

	void TcpConn::Close()
	{
		sock.close();
	}

	void TcpConn::Connect(std::string addr, int port)
	{
		TCPEndPoint ep(ip::address::from_string(addr.c_str()), port);
		sock.async_connect(ep, boost::bind(&TcpConn::connect_handler, this, _1));
	}

	void TcpConn::Start()
	{
		sock.async_read_some(buffer(readBuffer, 1024), boost::bind(&TcpConn::recv_handler, this, _1, _2));
	}


	void TcpConn::recv_handler(const error_code& ec, size_t)
	{
		if (ec && ec.value() != 2) {

		}
		else {
			sock.async_read_some(buffer(readBuffer, 1024), boost::bind(&TcpConn::recv_handler, this, _1, _2));
		}
	}

	void TcpConn::send_handler(const error_code& ec, size_t)
	{
		if (ec) {

		}
	}

	void TcpConn::connect_handler(const error_code& ec)
	{
		if (ec) {
			std::cout << "connect error" << std::endl;
		}
		else {
			std::cout << "connect succ" << std::endl;
		}
	}

}

