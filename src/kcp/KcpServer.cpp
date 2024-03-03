#include "KcpServer.h"
#include <iostream>

namespace AsioNet
{
	KcpServer::KcpServer(uint32_t id,io_ctx& ctx,IEventPoller* p)
	{

	}

	KcpServer::~KcpServer()
	{
	}


	bool KcpServer::Write(const char* data, size_t trans)
	{
        return true;
	}

	KcpKey KcpServer::Key()
	{
		if(m_key == 0){
		}
		return m_key;
	}

}

