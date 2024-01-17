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
	using _lock_guard_ = std::lock_guard<std::mutex>;

#define AN_INTERFACE

	struct AN_Msg {
		unsigned short len;
		// char data[0];
	};
	constexpr size_t AN_MSG_MAX_SIZE = (1 << (sizeof(AN_Msg::len) * 8)) - 1;

	using NetKey = unsigned long long;	// addr:port
    struct NetAddr{
        std::string ip;
        unsigned short port;
    };

	template<typename T>
	class Singleton {
	public:
		T* GetInstance()
		{
			static T m_;
			return &m_;
		}
	protected:
		Singleton() {}
	};

	
}
