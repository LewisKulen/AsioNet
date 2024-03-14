#include "KcpServer.h"
#include <iostream>

namespace AsioNet
{
	// ****************** TcpConnMgr *******************
	std::shared_ptr<KcpConn> KcpConnMgr::GetConn(NetKey k)
	{
		_lock_guard_(m_lock);
		if(m_conns.find(k) != m_conns.end()){
			return m_conns[k];
		}
		return nullptr;
	}
	void KcpConnMgr::DelConn(NetKey k)
	{
		_lock_guard_(m_lock);
		m_conns.erase(k);
	}
	
	// void KcpConnMgr::Disconnect(NetKey k)
	// {
	// 	_lock_guard_(m_lock);
	// 	auto itr = m_conns.find(k);
	// 	if (itr != m_conns.end()) {
	// 		itr->second->Close();
	// 	}
	// }
	void KcpConnMgr::AddConn(std::shared_ptr<KcpConn> conn)
	{
		_lock_guard_(m_lock);
		if(!conn){
			return;
		}
		if(m_conns.find(conn->Key()) == m_conns.end()){
			m_conns[conn->Key()] = conn;
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
	m_sock(ctx),ptr_poller(p),m_conv(0),m_port(0)
	{
	}

	KcpServer::~KcpServer()
	{
	}

	void KcpServer::Serve(uint16_t port,uint32_t conv)
	{
		if(m_port){
			return;
		}	
		UdpEndPoint ep(asio::ip::udp::v4(),port);
		m_sock.bind(ep);	// bind to local addr
		m_conv = conv;
		m_port = port;
		readLoop();
	}

	void KcpServer::readLoop()
	{
		static const uint32_t IKCP_OVERHEAD = 24;
		m_sock.async_receive_from(asio::buffer(m_kcpBuffer, sizeof(m_kcpBuffer)), m_tempRecevier,
        [self = shared_from_this()](const NetErr& ec, size_t trans){
            if (ec){
                return;
            }

			if(trans < sizeof(IKCP_OVERHEAD)){
				self->readLoop();
				return;
			}

			// 一个udp上面可能依赖了多个kcp，这里禁止客户端的这种行为
			// ip:port确定唯一客户端,符合认知
			if(self->m_conv != ikcp_getconv(self->m_kcpBuffer)){
				return;
			}

			NetKey key = (static_cast<unsigned long long>(self->m_tempRecevier.address().to_v4().to_uint()) << 32)
					| (static_cast<unsigned long long>(self->m_tempRecevier.port()) << 16)
					| static_cast<unsigned long long>(self->m_port/*listen port*/);

			auto conn = self->m_conns.GetConn(key);
			if(!conn){
				// 这里应该还有校验,不然这里如果被攻击了,那么就会一直创建conn,把服务器资源给爆了
				auto sock = std::make_shared<UdpSock>(self->m_sock.get_executor());
				conn = std::make_shared<KcpConn>(self->m_conv,sock,self->ptr_poller);
				self->m_conns.AddConn(conn);
			}

			// 问题:'断线重连',kcp的sn就直接从零来过了
			if(conn){
				conn->KcpInput(self->m_kcpBuffer,trans);
			}

            self->readLoop();
        });

	}
	
	void KcpServer::err_handler()
	{

	}

	bool KcpServer::Write(KcpKey,const char* data, size_t trans)
	{
        return true;
	}


}

