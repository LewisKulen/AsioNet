#include "KcpNetMgr.h"
#include "../utils/utils.h"

namespace AsioNet
{
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
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				} 
			}));
		}
	}

	KcpNetMgr::~KcpNetMgr()
	{
		// 通知线程退出
		m_isClose = true;
		m_ctx.stop();
		// 等待线程退出
		for(auto& th : thPool){
			th.join();
		}
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