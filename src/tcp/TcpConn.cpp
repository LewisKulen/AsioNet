#include "TcpConn.h"
#include <utility>	// std::move
#include "../utils/utils.h"

namespace AsioNet
{
	// by connect
	TcpConn::TcpConn(io_ctx& ctx, IEventPoller* p) :
		m_sock(ctx), ptr_poller(p)
	{
		init();
	}

	// by accept
	TcpConn::TcpConn(TcpSock&& sock, IEventPoller* p) :
		m_sock(std::move(sock)), ptr_poller(p)
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
		asio::ip::tcp::no_delay option(true);
		m_sock.set_option(option, ec);

		m_key = GenNetKey();
		ptr_owner = nullptr;
		m_close = false;	// 默认开启
	}

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
		// 可能有数据正在发送中，那么这次只要把数据放进去就行，借用write_handler进行继续发送
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
		if (head)	// 有数据就继续发
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

	// read实际就是单线程的运行的
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
				self->ptr_poller->PushRecv(self->Key(), self->m_readBuffer, trans);
				asio::async_read(self->m_sock, asio::buffer(self->m_readBuffer, sizeof(AN_Msg::len)),
					std::bind(&TcpConn::read_handler, self, std::placeholders::_1, std::placeholders::_2));
			});
	}

	// 错误处理
	void TcpConn::err_handler()	
	{
		// 关闭链接
		Close();
	}

	// 关闭连接，只会调用一次
	void TcpConn::Close()
	{
		{
			// 关闭了之后，可能还有其他异步操作残存在里面io_ctx里面
			// 当那些操作被取消后，都会进入error_handler中
			// 这里只Close一次，防止这些操作不停地向上抛Disconnect
			_lock_guard_(m_closeLock);
			if (m_close) {
				return;
			}
			m_close = true;
		}

		if (ptr_owner) {
			// 连接断开之后，外部最好彻底失去对conn的掌控			
			ptr_owner->DelConn(Key());
		}
		
		NetErr err;
		auto remote = m_sock.remote_endpoint(err);
		// 通知上层链接关闭了
		ptr_poller->PushDisconnect(Key(),remote.address().to_string(), remote.port());

		// 通知io_ctx，取消所有m_sock的异步操作
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	

		{
			// 如果write_handler出错调用Close释放了m_readBuffer，那么处在read_handler里面的m_readBuffer是不安全的
			// 对于m_readBuffer，其本身就是'单线程'跑，就采用定长数组不释放了，还能少加个锁
			_lock_guard_(m_sendLock);
			m_sendBuffer.Clear();
		}

		m_key = 0;
	}

	void TcpConn::Connect(const std::string& ip, uint16_t port, int retry)
	{
		TcpEndPoint ep(asio::ip::address::from_string(ip.c_str()), port);
		m_sock.async_connect(ep, [self = shared_from_this(), ip, port, retry](const NetErr& ec) {
			if (ec)
			{
				if (retry > 0)
				{
					self->Connect(ip, port, retry - 1);
					return;
				}
				return;
			}
			// 成功连接，通知上层
			if (self->ptr_owner) {
				self->ptr_owner->AddConn(self);
			}
			self->ptr_poller->PushConnect(self->Key(), ip, port);
			self->StartRead();
			});
	}

	void TcpConn::SetOwner(ITcpConnOwner* o)
	{
		ptr_owner = o;
	}

	TcpEndPoint TcpConn::Remote()
	{
		NetErr ne;
		return m_sock.remote_endpoint(ne);
	}

	NetKey TcpConn::Key()
	{
		return m_key;
	}


}

namespace AsioNet
{
	std::shared_ptr<TcpConn> TcpConnMgr::GetConn(NetKey k)
	{
		_lock_guard_(m_lock);
		if(m_conns.find(k) != m_conns.end()){
			return m_conns[k];
		}
		return nullptr;
	}
	void TcpConnMgr::DelConn(NetKey k)
	{
		_lock_guard_(m_lock);
		m_conns.erase(k);
	}
	
	void TcpConnMgr::Disconnect(NetKey k)
	{
		_lock_guard_(m_lock);
		auto itr = m_conns.find(k);
		if (itr != m_conns.end()) {
			itr->second->Close();
		}
	}
	void TcpConnMgr::AddConn(std::shared_ptr<TcpConn> conn)
	{
		_lock_guard_(m_lock);
		if(!conn){
			return;
		}
		if(m_conns.find(conn->Key()) == m_conns.end()){
			m_conns[conn->Key()] = conn;
		}
	}
	void TcpConnMgr::Broadcast(const char* data,size_t trans)
	{
		_lock_guard_(m_lock);
		for(auto p : m_conns){
			p.second->Write(data,trans);
		}
	}
	TcpConnMgr::~TcpConnMgr()
	{
		_lock_guard_(m_lock);
		for(auto p : m_conns){
			p.second->Close();
		}
	}

}