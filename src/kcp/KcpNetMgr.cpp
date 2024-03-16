#include "KcpNetMgr.h"
#include "../utils/utils.h"

namespace AsioNet
{
// ************************************************
	std::shared_ptr<KcpServer> KcpServerMgr::GetServer(ServerKey k)
	{
		_lock_guard_(m_lock);
		if(servers.find(k) != servers.end()){
			return servers[k];
		}
		return nullptr;
	}
	void KcpServerMgr::AddServer(std::shared_ptr<KcpServer> s)
	{
		_lock_guard_(m_lock);
		if(!s){
			return;
		}
		if(servers.find(s->Key()) == servers.end()){
			servers[s->Key()] = s;
		}
	}
	KcpServerMgr::~KcpServerMgr()
	{}

// ************************************************

	KcpNetMgr::KcpNetMgr(size_t th_num):m_isClose(false)
	{
		for (size_t i = 0; i < th_num; i++)
		{
			// std::move
			thPool.push_back(std::thread([self = this]{
				while(true){
					if (self->m_isClose)
					{
						break;
					}
					self->m_ctx.run();
					// 当ctx中没有任务的时候，m_ctx会变成stopped
					// 如果有任务，就会一直阻塞在run里面
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				} 
			}));
			thPool.back().detach();
		}
	}

	KcpNetMgr::~KcpNetMgr()
	{
		// 通知线程退出
		m_isClose = true;
	}

	void KcpNetMgr::Connect(IEventPoller* poller,const std::string& ip, uint16_t port,uint32_t conv)
	{
		auto conn = std::make_shared<KcpConn>(m_ctx, poller);
		// 连接并没有成功建立，这里不应该调用AddConn
		conn->SetOwner(&m_connMgr);
		conn->Connect(ip, port, conv);
	}

	ServerKey KcpNetMgr::Serve(IEventPoller* poller, const std::string& ip,uint16_t port, uint32_t conv)
	{
		auto s = std::make_shared<KcpServer>(m_ctx, poller);
		s->Serve(ip,port,conv);
		m_serverMgr.AddServer(s);
		return s->Key();
	}

	bool KcpNetMgr::Send(NetKey k, const char* data, size_t trans)
	{
		auto conn = m_connMgr.GetConn(k);
		if (conn) {
			return conn->Write(data, trans);
		}

		ServerKey sk = GetSvrKeyFromNetKey(k);
		auto svr = m_serverMgr.GetServer(sk);
		if(svr){
			auto cl = svr->GetConn(k);
			if (cl)
			{
				return cl->Write(data,trans);
			}
		}
		return false;
	}

	void KcpNetMgr::Broadcast(ServerKey sk, const char* data, size_t trans)
	{
		auto server = m_serverMgr.GetServer(sk);
		if (server) {
			server->Broadcast(data,trans);
		}
	}

	void KcpNetMgr::Disconnect(NetKey k)
	{
		m_connMgr.DelConn(k);

		ServerKey sk = GetSvrKeyFromNetKey(k);
		auto svr = m_serverMgr.GetServer(sk);
		if (svr) {
			svr->Disconnect(k);
		}
	}

}