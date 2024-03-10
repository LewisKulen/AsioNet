#pragma once

#include <asio.hpp>
#include <mutex>
#include <stdint.h>

namespace AsioNet {

	using io_ctx = asio::io_context;
	using NetErr = std::error_code;
	
	struct AN_Msg {
		uint16_t len;
		// char data[0];
	};

	constexpr size_t AN_MSG_MAX_SIZE = (1 << (sizeof(AN_Msg::len) * 8)) - 1;
	
	using NetKey = unsigned long long;	// addr:port
	using ServerKey = uint16_t;
	
	const NetKey INVALID_NET_KEY = 0;
	const ServerKey INVALID_SERVER_KEY = 0;

#define _lock_guard_(l) std::lock_guard<std::mutex>guard(l);

}
