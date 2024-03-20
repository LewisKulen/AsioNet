#include "TcpServer.h"
#include <utility>	// std::move
#include "../utils/utils.h"

namespace AsioNet
{
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

	void TcpServer::doAccept()
	{
		m_acceptor.async_accept([self = shared_from_this()](const NetErr& ec, TcpSock cli) {
			if (ec) { return; }

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

namespace AsioNet
{
	std::shared_ptr<TcpServer> TcpServerMgr::GetServer(ServerKey k)
	{
		_lock_guard_(m_lock);
		if(servers.find(k) != servers.end()){
			return servers[k];
		}
		return nullptr;
	}
	void TcpServerMgr::AddServer(std::shared_ptr<TcpServer> s)
	{
		_lock_guard_(m_lock);
		if(!s){
			return;
		}
		if(servers.find(s->Key()) == servers.end()){
			servers[s->Key()] = s;
		}
	}
	TcpServerMgr::~TcpServerMgr()
	{}
}