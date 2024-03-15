#include "TcpConn.h"
#include <utility>	// std::move

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
		makeKey();
	}

	TcpConn::~TcpConn()
	{
		Close();
	}

	void TcpConn::init()
	{
		NetErr ec;
		asio::ip::tcp::no_delay option(true);
		m_sock.set_option(option, ec);
		m_key = 0;
		ptr_owner = nullptr;
		m_close = false;	// 默认开启
	}

	// 保证连接建立之后，外部才能拿到conn,才能Write
	// 连接断开之后，外部将失去conn，从而无法write

	// 保证连接建立之后，外部才能拿到conn,才能Write
// 连接断开之后，外部将失去conn，从而无法write
	
	bool TcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}

		auto netLen = asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		_lock_guard_(m_sendLock);
		m_sendBuffer.Push((const char*)(&netLen), sizeof(AN_Msg::len));
		m_sendBuffer.Push(data, trans);
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			asio::async_write(m_sock, asio::buffer(head->buffer, head->wpos),
				std::bind(&TcpConn::write_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		return true;
	}

	void TcpConn::write_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler();
			return;
		}

		_lock_guard_(m_sendLock);
		m_sendBuffer.FreeDeatched();
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, asio::buffer(head->buffer, head->wpos),
				std::bind(&TcpConn::write_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		//else {
		//	// 可以考虑顺带释放一些发送缓冲区
		//  // 因为目前发送缓冲区大小是动态分配的，只会扩大，不会缩容
		//	// m_sendBuffer.Shrink();
		//}
	}

	void TcpConn::StartRead()
	{
		async_read(m_sock, asio::buffer(m_readBuffer, sizeof(AN_Msg::len)),
			bind(&TcpConn::read_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	// 同一时刻只会存在一个读/写异步任务存在
	void TcpConn::read_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler();
			return;
		}

		auto netLen = *((decltype(AN_Msg::len)*)m_readBuffer);
		auto hostLen = asio::detail::socket_ops::
			network_to_host_short(netLen);

		asio::async_read(m_sock, asio::buffer(m_readBuffer, hostLen),
			[self = shared_from_this()](const NetErr& ec, size_t trans) {
				if (ec)
				{
					self->err_handler();
					return;
				}
				self->ptr_poller->PushRecv(self->Key(),self->m_readBuffer, trans);
				asio::async_read(self->m_sock, asio::buffer(self->m_readBuffer, sizeof(AN_Msg::len)),
					std::bind(&TcpConn::read_handler, self, std::placeholders::_1, std::placeholders::_2));
			});
	}

	void TcpConn::err_handler()	// 关闭socket，错误输出
	{
		Close();	// 关闭链接
	}

	// 一旦被关闭了，这个TcpConn就应该直接释放掉,只能调用一次
	void TcpConn::Close()
	{
		{
			// 关闭了之后，可能还有其他异步操作残存在里面io_ctx里面
			// 当那些操作被取消后，都会进入error_handler中
			// 这里只Close一次，防止这些操作不停地向上抛Disconnect
			_lock_guard_(m_closeLock);
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
			// 如果write_handler出错调用Close释放了m_readBuffer，那么处在read_handler里面的m_readBuffer是不安全的
			// 对于m_readBuffer，其本身就是'单线程'跑，就采用定长数组不释放了，还能少加个锁
			_lock_guard_(m_sendLock);
			m_sendBuffer.Clear();
		}
		NetErr err;
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// 通知io_ctx，取消所有m_sock的异步操作
		m_key = 0;
	}

	NetKey TcpConn::Key()
	{
		return m_key;
	}

	void TcpConn::makeKey()
	{
			NetErr err;
			TcpEndPoint remote = m_sock.remote_endpoint(err);
			TcpEndPoint local = m_sock.local_endpoint(err);
			assert(!err);
			m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
					| (static_cast<unsigned long long>(remote.port()) << 16)
					| static_cast<unsigned long long>(local.port()/*listen port*/);
	}

	void TcpConn::Connect(std::string addr, uint16_t port,int retry)
	{
		TcpEndPoint ep(asio::ip::address::from_string(addr.c_str()), port);
		m_sock.async_connect(ep, [self = shared_from_this(), addr,port, retry](const NetErr& ec) {
				if (ec)
				{
					if (retry > 0)
					{
						self->Connect(addr,port,retry-1);
						return;
					}
					return;
				}
				self->makeKey();
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

