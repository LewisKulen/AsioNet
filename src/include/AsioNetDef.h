#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <utility>	// std::move
#include <mutex>


namespace AsioNet {

	using io_ctx = boost::asio::io_context;
	using TcpSock = boost::asio::ip::tcp::socket;
	using TcpEndPoint = boost::asio::ip::tcp::endpoint;

	using NetErr = boost::system::error_code;
	
#define _lock_guard_(l) std::lock_guard<std::mutex>guard(l);

	struct AN_Msg {
		unsigned short len;
		// char data[0];
	};
	constexpr size_t AN_MSG_MAX_SIZE = (1 << (sizeof(AN_Msg::len) * 8)) - 1;

	using NetKey = unsigned long long;	// addr:port
	using ServerKey = unsigned short;

	const NetKey INVALID_NET_KEY = 0;
	const ServerKey INVALID_SERVER_KEY = 0;

    struct NetAddr{
        std::string ip;
        unsigned short port;
    };

}
