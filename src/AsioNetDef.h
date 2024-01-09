#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <utility>	// std::move
#include <mutex>

extern std::mutex g_lock;

namespace AsioNet {

	using io_ctx = boost::asio::io_context;
	using TcpSock = boost::asio::ip::tcp::socket;
	using TcpEndPoint = boost::asio::ip::tcp::endpoint;

	using NetErr = boost::system::error_code;

#define AN_INTERFACE

	struct AN_Msg {
		unsigned short len;
		// char data[0];
	};
	constexpr size_t AN_MSG_MAX_SIZE = 1 << (sizeof(AN_Msg::len) * 8);

	using NetKey = unsigned long long;
	//struct MakeNetKey {
	//	NetKey operator()(const TcpEndPoint& ep)
	//	{
	//		return (static_cast<unsigned long long>(ep.address().to_v4().to_uint()) << 32)
	//				| static_cast<unsigned long long>(ep.port());
	//	}
	//};


}
