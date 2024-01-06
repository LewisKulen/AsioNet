#include "TcpConn.h"
#include <iostream>

namespace AsioNet {

	TcpConn::TcpConn(io_context& s) :sock_(s)
	{

	}

	bool TcpConn::Write(const char* data, size_t trans)
	{
		auto len = host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));
		if (len != trans)
		{
			return false;
		}
		std::lock_guard<std::mutex> guard(sendLock);
		sendBuffer.Push((const char*)(&len), sizeof(AN_Msg::len));
		sendBuffer.Push(data, trans);
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(sock_, buffer(head->buffer, head->pos), boost::bind(&TcpConn::write_handler, this, _1, _2));
		}
		return true;
		/*
		async_write_some
		buffers
			One or more data buffers to be written to the socket.
			Although the buffers object may be copied as necessary,
			ownership of the underlying memory blocks is retained by the caller,
			which must guarantee that they remain valid until the completion handler is called.
		Remarks
			The write operation may not transmit all of the data to the peer.
			Consider using the async_write function if you need to ensure that all data is written before the asynchronous operation completes.
		// 如果使用这个函数，那么我的发送缓冲区将写的有点复杂了
		*/
		// sock_.async_write_some(buffer(acData, iSize), boost::bind(&TcpConn::send_handler, this, _1, _2));
	}
	void TcpConn::write_handler(const error_code& ec, size_t)
	{
		if (ec) { std::cout << ec.message() << std::endl; return; }

		std::lock_guard<std::mutex> guard(sendLock);
		sendBuffer.FreeDeatched();
		if (!sendBuffer.Empty())
		{
			auto head = sendBuffer.DetachHead();
			if (head)
			{
				async_write(sock_, buffer(head->buffer, head->pos), boost::bind(&TcpConn::write_handler, this, _1, _2));
			}
		}
	}

	void TcpConn::StartRead()
	{
		// if sock_ is not open?
		async_read(sock_, buffer(readBuffer, sizeof(AN_Msg::len)), boost::bind(&TcpConn::read_head_handler, this, _1, _2));
	}

	void TcpConn::read_head_handler(const error_code& ec, size_t)
	{
		if (ec) { std::cout << ec.message() << std::endl; return; }

		auto len = *((decltype(AN_Msg::len)*)readBuffer);
		async_read(sock_, buffer(readBuffer, len), boost::bind(&TcpConn::read_body_handler, this, _1, _2));
	}

	void TcpConn::read_body_handler(const error_code& ec, size_t trans)
	{
		if (ec) { std::cout << ec.message() << std::endl; return; }
		std::cout << "recv[]" << ",data:" << std::string(readBuffer, trans) << std::endl;
		async_read(sock_, buffer(readBuffer, sizeof(AN_Msg::len)), boost::bind(&TcpConn::read_head_handler, this, _1, _2));
	}

	void TcpConn::Close()
	{
		sock_.shutdown(ip::tcp::socket::shutdown_both);
		sock_.close();
	}

}

