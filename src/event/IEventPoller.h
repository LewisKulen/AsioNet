#pragma once
#include "../include/AsioNetDef.h"

namespace AsioNet
{
    struct IEventPoller
	{
		virtual void PushAccept(NetKey k) = 0;
		virtual void PushConnect(NetKey k) = 0;
		virtual void PushDisconnect(NetKey k) = 0;
		virtual void PushRecv(NetKey k, const char *data, size_t trans) = 0;

		virtual ~IEventPoller(){}
	};
}