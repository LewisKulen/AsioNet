#include "TcpServer.h"
#include <utility>	// std::move
#include "../utils/utils.h"

namespace AsioNet
{
	// ****************** TcpConnMgr *******************
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

	// ************************************************************

	TcpServer::TcpServer(io_ctx& ctx,IEventPoller* p):
		m_acceptor(ctx),ptr_poller(p)
	{
		m_key = GenSvrKey();
	}

	TcpServer::~TcpServer()
	{
		NetErr err;
		m_acceptor.close(err);
	}

	void TcpServer::Serve(const std::string& ip,uint16_t port)
	{
		TcpEndPoint ep(asio::ip::address_v4().from_string(ip), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();
		doAccept();
	}

	// 当accept失败时，自动释放TcpServer资源
	void TcpServer::doAccept()
	{
		m_acceptor.async_accept([self = shared_from_this()](const NetErr& ec, TcpSock cli) {
			if (ec) { return; }

			// 当client conn断开连接时，自动释放conn资源
			auto conn = std::make_shared<TcpConn>(std::move(cli),self->ptr_poller);

			conn->SetOwner(&(self->connMgr));
			
			// 这里顺序不能错
			// 如果PushAccept之后立即Write，要保证此时connMgr里面有
			// PushAccept显然要在PushRecv之前，遂采用如下顺序
			self->connMgr.AddConn(conn);
			
			TcpEndPoint remote = conn->Remote();
			self->ptr_poller->PushAccept(conn->Key(), remote.address().to_string(),remote.port());
			conn->StartRead();

			self->doAccept();
		});
	}	

	void TcpServer::Broadcast(const char* data,size_t trans)
	{
		connMgr.Broadcast(data,trans);
	}

	std::shared_ptr<TcpConn> TcpServer::GetConn(NetKey k)
	{
		return connMgr.GetConn(k);
	}

	void TcpServer::Disconnect(NetKey k)
	{
		return connMgr.Disconnect(k);
	}

	ServerKey TcpServer::Key()
	{
		return m_key;
	}

}