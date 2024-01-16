#include "AsioNet.h"

namespace AsioNet
{
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

}