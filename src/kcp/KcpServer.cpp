#include "KcpServer.h"

namespace AsioNet
{
	// ****************** TcpConnMgr *******************

	void KcpConnMgr::DelConn(NetKey key)
	{
		_lock_guard_(m_lock);
		auto itr = m_conns.find(key);
		if (itr != m_conns.end())
		{
			auto conn = itr->second;
			m_conns.erase(conn->Key());
			m_connHelper.erase(conn->Remote());
		}
	}

	void KcpConnMgr::AddConn(std::shared_ptr<KcpConn> conn)
	{
		_lock_guard_(m_lock);
		if (!conn) {
			return;
		}
		if (m_conns.find(conn->Key()) == m_conns.end()) {
			m_conns[conn->Key()] = conn;
			m_connHelper[conn->Remote()] = conn->Key();
		}
	}
	
	std::shared_ptr<KcpConn> KcpConnMgr::GetConn(NetKey k)
	{
		_lock_guard_(m_lock);
		auto itr = m_conns.find(k);
		if(itr != m_conns.end()){
			return itr->second;
		}
		return nullptr;
	}

	std::shared_ptr<KcpConn> KcpConnMgr::GetConn(const UdpEndPoint& remote)
	{
		_lock_guard_(m_lock);
		auto itr = m_connHelper.find(remote);
		if (itr != m_connHelper.end()) {
			auto conn = m_conns.find(itr->second);
			if (conn != m_conns.end())
			{
				return conn->second;
			}
		}
		return nullptr;
	}

	void KcpConnMgr::Disconnect(NetKey k)
	{
		_lock_guard_(m_lock);
		auto itr = m_conns.find(k);
		if (itr != m_conns.end()) {
			itr->second->Close();
		}
	}

	void KcpConnMgr::Broadcast(const char* data,size_t trans)
	{
		_lock_guard_(m_lock);
		for(auto p : m_conns){
			p.second->Write(data,trans);
		}
	}

	KcpConnMgr::~KcpConnMgr()
	{
		_lock_guard_(m_lock);
		for(auto p : m_conns){
			p.second->Close();
		}
	}
	
	// ************************************************************


	KcpServer::KcpServer(io_ctx& ctx,IEventPoller* p):
	ptr_poller(p),m_conv(0)
	{
		m_sock = std::make_shared<UdpSock>(ctx);
		memset(m_kcpBuffer, 0, sizeof(m_kcpBuffer));
		m_key = GenSvrKey();
	}

	KcpServer::~KcpServer()
	{
	}

	void KcpServer::Serve(const std::string& ip,int16_t port,uint32_t conv)
	{
		if(m_conv){
			return;
		}	
		UdpEndPoint ep(asio::ip::address_v4().from_string(ip), port);
		m_sock->open(ep.protocol());
		m_sock->bind(ep);	// bind to local addr

		m_conv = conv;
		readLoop();
	}

	void KcpServer::readLoop()
	{
		m_sock->async_receive_from(asio::buffer(m_kcpBuffer, sizeof(m_kcpBuffer)), m_tempRecevier,
        [self = shared_from_this()](const NetErr& ec, size_t trans){
            if (ec){
                return;
            }

			if(trans < sizeof(IKCP_OVERHEAD) || trans > IKCP_MTU){
				self->readLoop();
				return;
			}

			// 一个udp上面可能依赖了多个kcp，这里禁止客户端的这种行为
			// ip:port确定唯一客户端,符合认知
			if(self->m_conv != ikcp_getconv(self->m_kcpBuffer)){
				return;
			}

			auto& remote = self->m_tempRecevier;
			auto conn = self->m_conns.GetConn(remote);

			if (!conn)
			{
				// 这里应该还有校验,不然这里如果被攻击了,那么就会一直创建conn,把服务器资源给爆了
				conn = std::make_shared<KcpConn>(self->m_sock, remote, self->ptr_poller, self->m_conv);
				conn->KcpUpdate();
				self->m_conns.AddConn(conn);

				self->ptr_poller->PushAccept(conn->Key(), remote.address().to_string(), remote.port());
			}
			
			if(conn){
				conn->KcpInput(self->m_kcpBuffer,trans);
			}

            self->readLoop();
        });

	}
	
	void KcpServer::err_handler()
	{
	}

	bool KcpServer::Write(NetKey key,const char* data, size_t trans)
	{
		auto conn = m_conns.GetConn(key);
		if (conn)
		{
			return conn->Write(data, trans);
		}
        return false;
	}

	ServerKey KcpServer::Key()
	{
		return m_key;
	}

	std::shared_ptr<KcpConn> KcpServer::GetConn(NetKey key)
	{
		return m_conns.GetConn(key);
	}

	void KcpServer::Broadcast(const char* data, size_t trans)
	{
		m_conns.Broadcast(data,trans);
	}

	void KcpServer::Disconnect(NetKey k)
	{
		return m_conns.Disconnect(k);
	}

}

