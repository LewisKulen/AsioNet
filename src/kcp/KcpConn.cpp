#include "KcpConn.h"
#include "../utils/utils.h"

namespace AsioNet
{
	KcpConn::KcpConn(std::shared_ptr<UdpSock> sock,const UdpEndPoint& remote,IEventPoller* p,uint32_t conv) :
		m_sock(sock),m_sender(remote),m_updater(sock->get_executor()), ptr_poller(p),m_conv(conv)
	{
		init();
		initKcp();
	}

	// client
	KcpConn::KcpConn(io_ctx& ctx,IEventPoller* p):
		m_updater(ctx), ptr_poller(p),m_conv(0)
	{
		m_sock = std::make_shared<UdpSock>(ctx);
		init();
	}

	KcpConn::~KcpConn()
	{
		Close();
	}

	void KcpConn::init()
	{
		m_key = GenNetKey();
		ptr_owner = nullptr;
	}

	void KcpConn::initKcp()
	{
		assert(m_conv);
        m_kcp = ikcp_create(m_conv,this/*user*/);
		m_kcp->output = &KcpConn::kcpOutPutFunc;	// ikcp_setoutput(m_kcp,&kcpOutPutFunc);
		ikcp_nodelay(m_kcp, 1, 2, 1, 0);
		ikcp_setmtu(m_kcp,IKCP_MTU);
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

	void KcpConn::Connect(const std::string& ip,uint16_t port,uint32_t conv)
	{
		if(m_conv){
			return;
		}
		m_conv = conv;
		m_sender = UdpEndPoint(asio::ip::address::from_string(ip.c_str()),port);
		m_sock->connect(m_sender);

		initKcp();
		m_kcp->output = &KcpConn::kcpOutPutFunc1;
		KcpUpdate();
		readLoop();

		// 这里可以做成发一个协议过去验证成功并收到返回了才算成功
		// 相当于要异步处理
		if(ptr_owner){
			ptr_owner->AddConn(shared_from_this());
		}
		ptr_poller->PushConnect(Key(),ip,port);
	}

	// ikcp_update的时候才会调用
	int KcpConn::kcpOutPutFunc(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		static NetErr ec;
		auto ptr = static_cast<KcpConn*>(user);
		// 这里不采用async_send的方式发送，因为kcp本身就已经是async的
		// 而且udp的发送本来就很快，再改成async_send不仅增加逻辑复杂性，性能可能更低
		ptr->m_sock->send_to(asio::buffer(buf, len), ptr->m_sender,0,ec);
		return !ec;
	}

	// 问题：
	// 读，写操作都需要对整个kcp加锁
	// 而kcp性能依赖于ikcp_update的实际效率
	void KcpConn::readLoop()
	{
		// udp包有界性，一次一定接受一个包，如果m_kcpBuffer不够，则只接受前面那段
		// server有他自己的readLoop
		m_sock->async_receive(asio::buffer(m_kcpBuffer, sizeof(m_kcpBuffer)),
        [self = shared_from_this()](const NetErr& ec, size_t trans){
            if (ec){
                self->err_handler();
                return;
            }

			if (trans < sizeof(IKCP_OVERHEAD) || trans > IKCP_MTU) {
				return;
			}
			
			self->KcpInput(self->m_kcpBuffer,trans);
			
            self->readLoop();
        });
	}

	// 客户端版使用
	int KcpConn::kcpOutPutFunc1(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		static NetErr ec;
		auto ptr = static_cast<KcpConn*>(user);
		ptr->m_sock->send(asio::buffer(buf, len),0,ec);
		return !ec;
	}

	void KcpConn::KcpInput(const char* data,size_t trans)
	{
		int recv = 0;
		{
			_lock_guard_(m_kcpLock);
			if (!m_kcp) {	// closed
				return;
			}

			ikcp_input(m_kcp, data, trans);

			// 这里其实用同一个buffer就行，input内部会把需要的数据拷贝走的，这里为了逻辑清楚点就用两个了
			// 尝试从kcp里面获取一个包
			recv = ikcp_recv(m_kcp, m_readBuffer, sizeof(m_readBuffer));
		}

		if(recv > 0){   
			ptr_poller->PushRecv(Key(),m_readBuffer,recv);
		}

		// 源码分析：ikcp_recv
		// if (peeksize > len) return -3;
		// 如果对端发了个基于kcp协议的很大的包，那么这个包就会一直卡在kcp_recv里面，之后的包将再也取不出来
		// 我这边的buffer如果不够大，那么接下来就再也取不出包了
		// 直接断开连接
		if (recv == -3){
			err_handler();
		}
	}
	// kcp-go用小根堆来管理多个kcp的update
	// 这里先用asio自带的，性能不够再做优化
	void KcpConn::KcpUpdate()
	{
		auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();

		uint32_t after = 0;	// 10ms

		{
			_lock_guard_(m_kcpLock);
			if (!m_kcp)
			{
				return;
			}

			// 正常一次check，一次update，然后再check获取下次update的时间
			// 这里直接用循环了
			ikcp_update(m_kcp, ticks);
			after = ikcp_check(m_kcp, ticks);
		}

		if (after <= ticks)	// 理论上不会出现这种情况，防止有bug
		{
			after = ticks + 10;
		}

		m_updater.expires_after(std::chrono::milliseconds(after - ticks));
		// asio本身就是用小根堆去处理定时任务的(网上说的，我没看过)，所以就不像kcp-go里面那样自己实现了
		m_updater.async_wait([self = shared_from_this()](const NetErr& ec) {
			if (ec)
			{
				self->err_handler();
				return;
			}

			self->KcpUpdate();
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
		ptr_poller->PushDisconnect(Key(),m_sender.address().to_string(),m_sender.port());// 通知上层链接关闭了

		m_updater.cancel();
		if(m_sock){
			NetErr err;
			m_sock->shutdown(asio::ip::tcp::socket::shutdown_both, err);
			m_sock->close(err);	// 通知io_ctx，取消所有m_sock的异步操作
		}
		m_key = 0;
	}

	NetKey KcpConn::Key()
	{
		return m_key;
	}

	UdpEndPoint KcpConn::Remote()
	{
		return m_sender;
	}

	void KcpConn::SetOwner(IKcpConnOwner* o)
	{
		ptr_owner = o;
	}
}

