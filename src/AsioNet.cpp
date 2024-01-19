#include "AsioNet.h"
#include "event/DefaultEventPoller.h"

#include <chrono>

namespace AsioNet
{
// ************************************************
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
		if(servers.find(s->GetKey()) == servers.end()){
			servers[s->GetKey()] = s;
		}
	}

// ************************************************

	TcpNetMgr::TcpNetMgr(size_t th_num, IEventHandler *h)
	{
		ptr_poller = new DefaultEventPoller(h);

		for (size_t i = 0; i < th_num; i++)
		{
			// std::move
			thPool.push_back(std::thread([self = this]{
				while(true){
					self->ctx.run();	// 只要有io事件，这个Run就不会返回
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				} 
			}));
		}
	}

	TcpNetMgr::~TcpNetMgr()
	{
		// 发送结束事件，通知线程退出
		// ctx.poll_one();
		for (auto &t : thPool)
		{
			t.join();
		}
		if (ptr_poller)
		{
			delete ptr_poller;
		}
	}

	void TcpNetMgr::Connect(std::string ip, unsigned short port /*,options*/)
	{
		auto conn = std::make_shared<TcpConn>(ctx,ptr_poller);
		// connMgr.AddConn(conn);	// 连接还没有真正建立
		conn->Connect(ip, port);
	}

	ServerKey TcpNetMgr::Serve(unsigned short port /*,options*/)
	{
		auto s = std::make_shared<TcpServer>(ctx);
		serverMgr.AddServer(s);
		s->Serve(port);
		return s->GetKey();
	}

	bool TcpNetMgr::Send(NetKey k, const char *data, size_t trans)
	{
		auto conn = connMgr.GetConn(k);
		if(conn){
			return conn->Write(data,trans);
		}
		return false;
	}

	bool TcpNetMgr::ServerSend(NetKey k, const char* data, size_t trans)
	{
		ServerKey sk = k & 0xff;
		auto server = serverMgr.GetServer(sk);
		if(server){
			auto client = server->GetConn(k);
			if (client)
			{
				return client->Write(data,trans);
			}
		}
		return false;
	}
}