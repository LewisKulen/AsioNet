#include "Tool.h"

namespace AsioNet
{
	inline NetKey MakeNetKey(const TcpEndPoint& ep)
	{
		if (ep.address().is_v4())
		{
			return (static_cast<unsigned long long>(ep.address().to_v4().to_uint()) << 32)
				| static_cast<unsigned long long>(ep.port());
		}

		if (ep.address().is_v6())
		{
			return 0;	// Ôİ²»Ö§³Ö
			//return (static_cast<unsigned long long>(ep.address().to_v4().to_uint()) << 32)
			//	| static_cast<unsigned long long>(ep.port());
		}
		return 0;
	}
}
