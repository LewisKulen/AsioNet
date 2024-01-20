#include "TcpConn.h"

namespace AsioNet
{
	// by connect
	TcpConn::TcpConn(io_ctx& ctx,IEventPoller* p) :
		m_sock(ctx),ptr_poller(p)
	{
		init();
	}

	// by accept
	TcpConn::TcpConn(TcpSock&& sock,IEventPoller* p) :
		m_sock(std::move(sock)),ptr_poller(p)
	{
		init();
	}

	TcpConn::~TcpConn()
	{
		Close();
	}

	void TcpConn::init()
	{
		NetErr ec;
		boost::asio::ip::tcp::no_delay option(true);
		m_sock.set_option(option, ec);
		m_key = 0;
		ptr_owner = nullptr;
		m_close = false;	// 默认开启
	}

	// 保证连接建立之后，外部才能拿到conn,才能Write
	// 连接断开之后，外部将失去conn，从而无法write
	bool TcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}

		auto netLen = boost::asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		_lock_guard_(sendLock);
		sendBuffer.Push((const char*)(&netLen), sizeof(AN_Msg::len));
		sendBuffer.Push(data, trans);
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		return true;
	}

	void TcpConn::write_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}

		_lock_guard_(sendLock);
		sendBuffer.FreeDeatched();
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		//else {
		//	// 可以考虑顺带释放一些发送缓冲区
		//  // 因为目前发送缓冲区大小是动态分配的，只会扩大，不会缩容
		//	// sendBuffer.Shrink();
		//}
	}

	void TcpConn::StartRead()
	{
		async_read(m_sock, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
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

		auto netLen = *((decltype(AN_Msg::len)*)readBuffer);
		auto hostLen = boost::asio::detail::socket_ops::
			network_to_host_short(netLen);

		async_read(m_sock, boost::asio::buffer(readBuffer, hostLen),
			[self = shared_from_this()](const NetErr& ec, size_t trans) {
				if (ec)
				{
					self->err_handler(ec);
					return;
				}
				self->ptr_poller->PushRecv(self->Key(),self->readBuffer, trans);
				async_read(self->m_sock, boost::asio::buffer(self->readBuffer, sizeof(AN_Msg::len)),
					boost::bind(&TcpConn::read_handler, self, boost::placeholders::_1, boost::placeholders::_2));
			});
	}

	void TcpConn::err_handler(const NetErr& err)	// 关闭socket，错误输出
	{
		ptr_poller->PushError(Key(), err);

		Close();	// 关闭链接
	}

	// 一旦被关闭了，这个TcpConn就应该直接释放掉,只能调用一次
	void TcpConn::Close()
	{
		{
			// 关闭了之后，可能还有其他异步操作残存在里面io_ctx里面
			// 当那些操作被取消后，都会进入error_handler中
			// 这里只Close一次，防止这些操作不停地向上抛Disconnect
			_lock_guard_(closeLock);
			if(m_close){	
				return;
			}
			m_close = true;
		}

		if (ptr_owner) {
			// 连接断开之后，外部需要彻底失去对conn的掌控，这样的话conn就会自动消亡
			ptr_owner->DelConn(Key());
		}
		ptr_poller->PushDisconnect(Key());// 通知上层链接关闭了

		{
			// 如果write_handler出错调用Close释放了readBuffer，那么处在read_handler里面的readBuffer是不安全的
			// 对于readBuffer，其本身就是'单线程'跑，就采用定长数组不释放了，还能少加个锁
			_lock_guard_(sendLock);
			sendBuffer.Clear();
		}
		NetErr err;
		m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// 通知io_ctx，取消所有m_sock的异步操作
		m_key = 0;
	}

	NetKey TcpConn::Key()
	{
		if(m_key == 0){
			NetErr err;
			TcpEndPoint remote = m_sock.remote_endpoint(err);
			TcpEndPoint local = m_sock.local_endpoint(err);
			if(!err){
				m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
						| (static_cast<unsigned long long>(remote.port()) << 16)
						| static_cast<unsigned long long>(local.port()/*listen port*/);
			}
		}
		return m_key;
	}

	void TcpConn::Connect(std::string addr, unsigned short port,int retry)
	{
		TcpEndPoint ep(boost::asio::ip::address::from_string(addr.c_str()), port);
		m_sock.async_connect(ep, [self = shared_from_this(), addr,port, retry](const NetErr& ec) {
				if (ec)
				{
					self->ptr_poller->PushError(self->Key(), ec);
					if (retry > 0)
					{
						self->Connect(addr,port,retry-1);
						return;
					}
					return;
				}
				// 只有成功建立了之后，外部才能拿到conn
				// 这里的顺序同accept_handler
				if (self->ptr_owner) {
					self->ptr_owner->AddConn(self);	// 外部不应该保存多份
				}
				self->ptr_poller->PushConnect(self->Key());
				self->StartRead();
			});
	}

	void TcpConn::SetOwner(ITcpConnOwner* o)
	{
		ptr_owner = o;
	}
}

