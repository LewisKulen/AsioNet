#include "KcpConn.h"
#include <iostream>

namespace AsioNet
{
    void kcpOutPutFunc(const char *buf, int len,ikcpcb *kcp, void *user)
    {
        
    }

	KcpConn::KcpConn(io_ctx& ctx,IEventPoller* p) :
		m_sock(ctx),ptr_poller(p)
	{
        m_kcp = ikcp_create(100,this);
        ikcp_setoutput(m_kcp,&kcpOutPutFunc);
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
		m_close = false;	// 默认开启
	}

	// 保证连接建立之后，外部才能拿到conn,才能Write
	// 连接断开之后，外部将失去conn，从而无法write
	bool KcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}
        ikcp_send(m_kcp, data, trans);

		auto netLen = boost::asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		_lock_guard_(m_sendLock);
		m_sendBuffer.Push((const char*)(&netLen), sizeof(AN_Msg::len));
		m_sendBuffer.Push(data, trans);
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			m_sock.async_send(boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&KcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		return true;
	}

	void KcpConn::write_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}
        printf("send to:[%s][%d] succ\n",
        m_sock.remote_endpoint().address().to_string().c_str(),
        m_sock.remote_endpoint().port());

		_lock_guard_(m_sendLock);
		m_sendBuffer.FreeDeatched();
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			m_sock.async_send(boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&KcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		//else {
		//	// 可以考虑顺带释放一些发送缓冲区
		//  // 因为目前发送缓冲区大小是动态分配的，只会扩大，不会缩容
		//	// m_sendBuffer.Shrink();
		//}
	}

	void KcpConn::ReadLoop()
	{
		m_sock.async_receive(boost::asio::buffer(m_readBuffer, sizeof(m_readBuffer)),
        [self = shared_from_this()](const NetErr& ec, size_t trans){
            if (ec)
            {
                self->err_handler(ec);
                return;
            }

            ikcp_input(self->m_kcp,self->m_readBuffer,trans);   // 这里用同一个就行，input内部会把需要的数据拷贝走的
            int recv = ikcp_recv(self->m_kcp,self->m_readBuffer,sizeof(self->m_readBuffer));

            // 如果recv < 0,说明收到的不是kcp协议
            if(recv > 0){   
                
            }
            self->ReadLoop();
        });
	}

	void KcpConn::err_handler(const NetErr& err)	// 关闭socket，错误输出
	{
		ptr_poller->PushError(Key(), err);

		Close();	// 关闭链接
	}

	// 一旦被关闭了，这个KcpConn就应该直接释放掉,只能调用一次
	void KcpConn::Close()
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
        if (m_kcp) ikcp_release(m_kcp);
		NetErr err;
		m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// 通知io_ctx，取消所有m_sock的异步操作
		m_key = 0;
	}

	NetKey KcpConn::Key()
	{
		if(m_key == 0){
			NetErr err;
			// TcpEndPoint remote = m_sock.remote_endpoint(err);
			// TcpEndPoint local = m_sock.local_endpoint(err);
			// if(!err){
			// 	m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
			// 			| (static_cast<unsigned long long>(remote.port()) << 16)
			// 			| static_cast<unsigned long long>(local.port()/*listen port*/);
			// }
		}
		return m_key;
	}

	void KcpConn::SetOwner(IKcpConnOwner* o)
	{
		ptr_owner = o;
	}
}

