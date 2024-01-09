#include "TcpConn.h"

namespace AsioNet
{
	TcpConn::TcpConn(io_ctx& ctx) : sock_(ctx)
	{
		init();
	}

	TcpConn::TcpConn(TcpSock &&sock) : sock_{std::move(sock)}
	{
		init();
	}

	TcpConn::~TcpConn()
	{
		NetErr err;
		sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		sock_.close(err);
	}

	void TcpConn::init()
	{
		boost::asio::ip::tcp::no_delay option(true);
		NetErr ec;
		sock_.set_option(option, ec);
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
				   boost::bind(&TcpConn::read_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	// 同一时刻只会存在一个读/写异步任务存在
	void TcpConn::read_handler(const NetErr& ec, size_t)
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
					[self = shared_from_this()](const NetErr &ec, size_t trans){
						if (ec)
						{
							self->err_handler(ec);
							return;
						}
						self->net_proc(self->readBuffer, trans);
						async_read(self->sock_, boost::asio::buffer(self->readBuffer, sizeof(AN_Msg::len)),
				   			boost::bind(&TcpConn::read_handler, self, boost::placeholders::_1, boost::placeholders::_2));
					});
	}

	void TcpConn::net_proc(const char* data, size_t trans)
	{

#ifdef _AN_PROC_IN_IO_THREAD_

#else

#endif

	}

	void TcpConn::err_handler(const NetErr& err)	// 关闭socket，错误输出
	{
		Close();	// 关闭链接
		// 通知上层链接关闭
	}
	
	void TcpConn::Close()
	{
		{
			std::lock_guard<std::mutex> guard(sendLock);
			sendBuffer.Clear();
		}
		// 如果write_handler出错调用Close释放了readBuffer，那么处在read_handler里面的readBuffer是不安全的
		// 对于readBuffer，就采用定长吧，不然代码写起来会有点丑感觉，需要分开处理写错误和读错误，但是我对他们的处理方式都是关闭链接，释放资源
		NetErr err;
		sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		sock_.close(err);
	}

}
