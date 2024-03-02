#include "./utils.h"

namespace AsioNet {
	NetAddr NetKey2Addr(NetKey key)
	{
		NetAddr res;
		unsigned ip = key >> 32;
		res.ip = asio::ip::make_address_v4(ip).to_string();
		res.port = static_cast<uint16_t>((key & 0xffff0000) >> 16);
		return res;
	}

	ServerKey NetKey2ServerKey(NetKey key)
	{
		return key & 0xffff;
	}
}
