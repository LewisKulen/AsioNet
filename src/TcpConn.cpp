#include "TcpConn.h"
#include <boost/bind/bind.hpp>
#include <utility>	// std::move

namespace AsioNet
{
	TcpConn::TcpConn(io_ctx& ctx) : sock_(ctx)
	{
	}

	TcpConn::TcpConn(TcpSock &&sock) : sock_{std::move(sock)}
	{
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
	}

	void TcpConn::write_handler(const NetErr &ec, size_t)
	{
		if (ec)
		{
			std::lock_guard<std::mutex> guard(sendLock);
			// sendBuffer.FreeDeatched();
			sendBuffer.Clear();
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
		// if sock_ is not open,this will get an error
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
		// net_proc(readBuffer, trans);
		async_read(sock_, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
				   boost::bind(&TcpConn::read_head_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	void TcpConn::Close()
	{
		NetErr err;
		sock_.close(err);	
		if (err)
		{
			if (logger) {
				logger->Log(sock_, err);
			}
		}
		
	}

	void TcpConn::err_handler(const NetErr& err)	// ¹Ø±Õsocket£¬´íÎóÊä³ö
	{
		// ´íÎóÊä³ö
		if (logger) {
			logger->Log(sock_,err);
		}
		Close();
	}
}
