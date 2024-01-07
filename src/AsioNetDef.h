#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <mutex>
extern std::mutex g_lock;

namespace AsioNet {
	// make sure is x64
	//static_assert(sizeof(void*) == 8);
	//using int8 = short;
	//using uint8 = unsigned char;
	//using int16 = short;
	//using uint16 = unsigned short;
	//using int32 = int;
	//using uint32 = unsigned int;
	//using int64 = long long int;
	//using uint64 = unsigned long long int;

	/*
	类名:大驼峰命名
	变量名:type大驼峰命名
	成员变量:m_变量名
	模板参数变量:V_大写
	模板参数:大写
	宏:大写_大写

	type说明：
	int32 整形，不管位数
	uint32 无符号整形，不管位数
	k 结构体/类
	f float
	lf doubl
	*/
	//using namespace boost::asio;
	//using namespace boost::system;
	//using namespace boost::asio::detail::socket_ops;
	using io_ctx = boost::asio::io_context;
	using TcpSock = boost::asio::ip::tcp::socket;
	using TcpEndPoint = boost::asio::ip::tcp::endpoint;

	using NetErr = boost::system::error_code;
	
#define AN_INTERFACE
#define AN_DEFAULT_HANDLER
	struct AN_Msg {
		unsigned short len;
		// char data[0];
	};
	constexpr size_t AN_MSG_MAX_SIZE = 1 << (sizeof(AN_Msg::len) * 8);

}
