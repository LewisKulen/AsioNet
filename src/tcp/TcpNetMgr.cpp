#include "TcpNetMgr.h"
#include "../utils/utils.h"

namespace AsioNet
{
	TcpNetMgr::TcpNetMgr(size_t th_num) :m_isClose(false)
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
					// 只要有事件在里面，这个run就不会返回，除非stop
					self->m_ctx.run();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				} 
			}));
		}
	}

	TcpNetMgr::~TcpNetMgr()
	{
		// 通知线程退出
		m_isClose = true;
		m_ctx.stop();
		// 等待线程退出
		for(auto& th : thPool){
			th.join();
		}
	}

	void TcpNetMgr::Connect(IEventPoller* poller,const std::string& ip, uint16_t port,int retry/*,options*/)
	{
		auto conn = std::make_shared<TcpConn>(m_ctx, poller);
		// 连接并没有成功建立，这里不应该调用AddConn
		conn->SetOwner(&m_connMgr);
		conn->Connect(ip, port, retry);
	}

	ServerKey TcpNetMgr::Serve(IEventPoller* poller, const std::string& ip,uint16_t port /*,options*/)
	{
		auto s = std::make_shared<TcpServer>(m_ctx, poller);
		s->Serve(ip,port);
		m_serverMgr.AddServer(s);
		return s->Key();
	}

	bool TcpNetMgr::Send(NetKey k, const char* data, size_t trans)
	{
		auto conn = m_connMgr.GetConn(k);
		if (conn) {
			return conn->Write(data, trans);
		}

		ServerKey svr = GetSvrKeyFromNetKey(k);
		auto server = m_serverMgr.GetServer(svr);
		if(server){
			auto client = server->GetConn(k);
			if (client)
			{
				return client->Write(data,trans);
			}
		}
		return false;
	}

	void TcpNetMgr::Broadcast(ServerKey sk, const char* data, size_t trans)
	{
		auto server = m_serverMgr.GetServer(sk);
		if (server) {
			server->Broadcast(data,trans);
		}
	}

	void TcpNetMgr::Disconnect(NetKey k)
	{
		m_connMgr.DelConn(k);

		ServerKey sk = GetSvrKeyFromNetKey(k);
		auto svr = m_serverMgr.GetServer(sk);
		if (svr) {
			svr->Disconnect(k);
		}
	}

}