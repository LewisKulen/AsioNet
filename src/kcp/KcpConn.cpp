#include "./KcpServer.h"
#include <iostream>

namespace AsioNet
{
	KcpConn::KcpConn(uint32_t id,io_ctx& ctx,IEventPoller* p) :
		m_sock(ctx),ptr_poller(p)
	{
        m_kcp = ikcp_create(id,this/*user*/);
		m_kcp->output = &KcpConn::kcpOutPutFunc;	// ikcp_setoutput(m_kcp,&kcpOutPutFunc);

		ikcp_nodelay(m_kcp, 1, 2, 1, 0);

		init();
	}

	KcpConn::~KcpConn()
	{
		Close();
	}

	void KcpConn::init()
	{
		m_key = 0;
		ptr_owner = nullptr;
	}

	bool KcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}

		// 只是将数据放到kcp的发送缓冲区里面，实际的发送在ikcp_update才会有实际的发送
		_lock_guard_(m_kcpLock);
		if (!m_kcp)
		{
			return false;
		}
		
        return ikcp_send(m_kcp, data, trans) > 0;
	}

	// ikcp_update的时候才会调用
	int KcpConn::kcpOutPutFunc(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		static NetErr ec;	// 仅为了不要抛异常
		auto ptr = static_cast<KcpConn*>(user);
		// 这里不采用async_send的方式发送，因为kcp本身就已经是async的
		// 而且udp的发送本来就很快，再改成async_send不仅增加逻辑复杂性，性能可能更低
		return ptr->m_sock.send(asio::buffer(buf, len), 0, ec);
	}


	// 问题：
	// 读，写操作都需要对整个kcp加锁
	// 而kcp性能依赖于ikcp_update的实际效率
	void KcpConn::StartRead()
	{
		// udp包有界性，一次一定接受一个包，如果m_kcpBuffer不够，则只接受前面那段
		// 问题：如果对端一直发包，会不会拖垮整个update的性能
		m_sock.async_receive(asio::buffer(m_kcpBuffer, sizeof(m_kcpBuffer)),
        [self = shared_from_this()](const NetErr& ec, size_t trans){
            if (ec)
            {
                self->err_handler();
                return;
            }

			int recv = 0;
			{
				_lock_guard_(self->m_kcpLock);
				if (!self->m_kcp) {	// closed
					return;
				}

				ikcp_input(self->m_kcp, self->m_kcpBuffer, trans);

				// 这里其实用同一个buffer就行，input内部会把需要的数据拷贝走的，这里为了逻辑清楚点就用两个了
				// 尝试从kcp里面获取一个包
				recv = ikcp_recv(self->m_kcp, self->m_readBuffer, sizeof(self->m_readBuffer));
			}

			if(recv > 0){   
				self->ptr_poller->PushRecv(self->Key(),self->m_readBuffer,recv);
			}

			// 源码分析：ikcp_recv
			// if (peeksize > len) return -3;
			// 如果对端发了个基于kcp协议的很大的包，那么这个包就会一直卡在kcp_recv里面，之后的包将再也取不出来
			// 我这边的buffer如果不够大，那么接下来就再也取不出包了
			// 直接断开连接
			if (recv == -3)
			{
				self->err_handler();
				return;
			}

            self->StartRead();
        });
	}

	// 网络库的错误处理：关闭连接
	void KcpConn::err_handler()
	{
		Close();
	}

	// 一旦被关闭了，这个KcpConn就应该直接释放掉,只调用一次
	void KcpConn::Close()
	{
		{
			_lock_guard_(m_kcpLock);
			if (m_kcp)
			{
				return;
			}
			
			ikcp_release(m_kcp);
			m_kcp = nullptr;
		}

		if (ptr_owner) {
			// 连接断开之后，外部需要彻底失去对conn的掌控，这样的话conn就会自动消亡
			ptr_owner->DelConn(Key());
		}
		ptr_poller->PushDisconnect(Key());// 通知上层链接关闭了

		NetErr err;
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// 通知io_ctx，取消所有m_sock的异步操作
		m_key = 0;
	}

	KcpKey KcpConn::Key()
	{
		if(m_key == 0){
			NetErr err;
			UdpEndPoint remote = m_sock.remote_endpoint(err);
			// TcpEndPoint local = m_sock.local_endpoint(err);
			if(!err){
				m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
						| (static_cast<unsigned long long>(remote.port()) << 16);
						// | static_cast<unsigned long long>(local.port()/*kcp_id*/);
			}
		}
		return m_key;
	}

	void KcpConn::SetOwner(IKcpConnOwner* o)
	{
		ptr_owner = o;
	}

	void KcpConn::kcpUpdateFunc()
	{
		auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>
						(std::chrono::system_clock::now().time_since_epoch()).count();

		_lock_guard_(m_kcpLock);
		if(!m_kcp){
			return;
		}
		ikcp_update(m_kcp,ticks);	// 负责ikcp_send等
	}
}

