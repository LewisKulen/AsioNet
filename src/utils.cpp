#pragma once
#include "utils.h"

namespace AsioNet
{
    NetAddr NetKey2Addr(NetKey key)
	{
		NetAddr res;
		res.port = static_cast<unsigned short>(key & 0xff00);
		unsigned ip = key >> 32;
		res.ip = boost::asio::ip::make_address_v4(ip).to_string();
		return res;
	}
	ServerKey NetKey2ServerKey(NetKey key)
	{
		return key & 0xff;
	}
}