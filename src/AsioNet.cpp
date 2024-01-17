#include "AsioNet.h"
#include "TcpServer.h"

#include <chrono>

namespace AsioNet
{
	TcpNetMgr::TcpNetMgr(size_t th_num/*线程数量*/)
	{
		for (size_t i = 0; i < th_num; i++)
		{
			// std::move
			th_pool.push_back(std::thread([self = this]{
				while(true){
					self->ctx.run();	// 只要有io事件，这个Run就不会返回
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}));
		}
		
	}
	TcpNetMgr::~TcpNetMgr()
	{
		for(auto& t : th_pool)
		{
			t.join();
		}
	}

	void TcpNetMgr::Connect(std::string ip, unsigned short port/*,options*/)
	{
		auto conn = std::make_shared<TcpConn>(ctx);
		conn->Connect(ip, port);
	}

	void TcpNetMgr::Serve(unsigned short port/*,options*/)
	{
		auto s = std::make_shared<TcpServer>(ctx);
		s->Serve(port);
	}

	bool TcpNetMgr::Send(NetKey, const char* data, size_t trans)
	{
		return false;
	}

	
	NetAddr NetKey2Addr(const NetKey& key)
	{
		NetAddr res;
		res.port = static_cast<unsigned short>(key & 0xff);
		unsigned ip = key >> 32;
		res.ip = boost::asio::ip::make_address_v4(ip).to_string();
		return res;
	}
}