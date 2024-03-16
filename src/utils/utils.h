#pragma once

#include "./AsioNetDef.h"

namespace AsioNet {
	NetKey GenNetKey(ServerKey svr = 0);
	ServerKey GenSvrKey();
	ServerKey GetSvrKeyFromNetKey(NetKey);
}
