#pragma once

#include <asio.hpp>
#include <mutex>

namespace AsioNet {

	using io_ctx = asio::io_context;
	using TcpSock = asio::ip::tcp::socket;
	using TcpEndPoint = asio::ip::tcp::endpoint;

	using UdpEndPoint = asio::ip::udp::endpoint;
	using UdpSock = asio::ip::udp::socket;
	using NetErr = std::error_code;
	
#define _lock_guard_(l) std::lock_guard<std::mutex>guard(l);

	struct AN_Msg {
		unsigned short len;
		// char data[0];
	};
	constexpr size_t AN_MSG_MAX_SIZE = (1 << (sizeof(AN_Msg::len) * 8)) - 1;
	const size_t AN_KCP_BUFFER_SIZE = 4096;

	using NetKey = unsigned long long;	// addr:port
	using ServerKey = unsigned short;
	
	const NetKey INVALID_NET_KEY = 0;
	const ServerKey INVALID_SERVER_KEY = 0;

    struct NetAddr{
        std::string ip;
        unsigned short port;
    };

	enum class NetErrCode {
		KCP_RECV_TOO_BIG = 1,
	};
}
