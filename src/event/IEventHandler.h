#pragma once

#include "../include/AsioNetDef.h"

namespace AsioNet
{
    struct IEventHandler
	{
		virtual void AcceptHandler(NetKey) = 0;
		virtual void ConnectHandler(NetKey) = 0;
		virtual void DisconnectHandler(NetKey) = 0;
		virtual void RecvHandler(NetKey, const char *data, size_t trans) = 0;
		virtual void RecvHandler(NetKey, const std::string &) = 0;
		virtual void ErrorHandler(NetKey,const NetErr&) = 0;
		virtual ~IEventHandler() {}
	};
}