#pragma once
#include "../utils/AsioNetDef.h"

namespace AsioNet
{
    struct IEventPoller
	{
		virtual void PushAccept(NetKey k,const std::string& ip,uint16_t port) = 0;
		virtual void PushConnect(NetKey k, const std::string& ip, uint16_t port) = 0;
		virtual void PushDisconnect(NetKey k, const std::string& ip, uint16_t port) = 0;
		virtual void PushRecv(NetKey k, const char *data, size_t trans) = 0;

		virtual ~IEventPoller(){}
	};
}