#pragma once

#include "../include/AsioNetDef.h"

namespace AsioNet {
	struct NetAddr {
		std::string ip;
		uint16_t port;
	};

	NetAddr NetKey2Addr(NetKey key);

	ServerKey NetKey2ServerKey(NetKey key);
}
