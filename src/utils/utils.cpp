#include "./utils.h"

namespace AsioNet 
{
	NetKey GenNetKey(ServerKey svr)
	{
		static NetKey id = AN_START_TIME % 666 + 666666;
		static std::mutex lock;

		_lock_guard_(lock);
		if (id == UINT32_MAX){
			id = 1;
		}
		++id;
		if (svr){
			return (static_cast<uint64_t>(svr) << 32) | id;
		}
		return id;
	}

	ServerKey GenSvrKey()
	{
		static ServerKey id = AN_START_TIME % 888 + 88888888;
		static std::mutex lock;

		_lock_guard_(lock);
		++id;
		return id;
	}

	ServerKey GetSvrKeyFromNetKey(NetKey key)
	{
		return static_cast<ServerKey>(key >> 32);
	}
}
