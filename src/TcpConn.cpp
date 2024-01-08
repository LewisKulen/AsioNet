#include "TcpConn.h"
#include <boost/bind/bind.hpp>

#include <iostream>
#include <utility>	// std::move
#include <string>

namespace AsioNet
{
	using namespace boost::placeholders;
	void out_err_handler(const NetErr &ec)
	{
		printf("error:%s\n", ec.message().c_str());
	}
	void out_net_proc(const char *data, size_t trans)
	{
		printf("recv[%lld],data[%s]\n", trans, std::string(data, trans).c_str());
	}

	TcpConn::TcpConn(io_ctx& ctx) : sock_(ctx)
	{
		err_handler = out_err_handler;
		net_proc = out_net_proc;
	}

	TcpConn::TcpConn(TcpSock &&sock) : sock_{std::move(sock)}
	{
		err_handler = out_err_handler;
		net_proc = out_net_proc;
	}

	TcpConn::~TcpConn()
	{
		Close();
	}
	bool TcpConn::Write(const char *data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE)
		{
			return false;
		}
		if (!sock_.is_open())
		{
			return false;
		}
		auto netLen = boost::asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		std::lock_guard<std::mutex> guard(sendLock);
		sendBuffer.Push((const char *)(&netLen), sizeof(AN_Msg::len));
		sendBuffer.Push(data, trans);
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(sock_, boost::asio::buffer(head->buffer, head->pos),
						boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
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
		// sock_.async_write_some(buffer(acData, iSize), boost::bind(&TcpConn::send_handler, this, boost::placeholders::_1, boost::placeholders::_2));
	}
	void TcpConn::write_handler(const NetErr &ec, size_t)
	{
		if (ec)
		{
			// 如果出错，那么就会导致数据不会再发成功，
			// 必须得FreeDeatched之后才能发送成功，如何处理错误好些呢
			err_handler(ec);
			return;
		}

		std::lock_guard<std::mutex> guard(sendLock);
		sendBuffer.FreeDeatched();
		if (!sendBuffer.Empty())
		{
			auto head = sendBuffer.DetachHead();
			if (head)
			{
				async_write(sock_, boost::asio::buffer(head->buffer, head->pos),
							boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
			}
		}
	}

	void TcpConn::StartRead()
	{
		// if sock_ is not open,will get error
		async_read(sock_, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
				   boost::bind(&TcpConn::read_head_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void TcpConn::read_head_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}

		auto netLen = *((decltype(AN_Msg::len) *)readBuffer);
		auto hostLen = boost::asio::detail::socket_ops::
			network_to_host_short(netLen);

		async_read(sock_, boost::asio::buffer(readBuffer, hostLen),
				   boost::bind(&TcpConn::read_body_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void TcpConn::read_body_handler(const NetErr &ec, size_t trans)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}
		net_proc(readBuffer, trans);
		async_read(sock_, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
				   boost::bind(&TcpConn::read_head_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void TcpConn::Close()
	{
		if (sock_.is_open())
		{
			sock_.shutdown(TcpSock::shutdown_both);
			sock_.close();	//close twice will occur err
		}
	}

}
