#pragma once

#include "AsioNetDef.h"
#include "BlockBuffer.h"
#include "Logger.h"

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	// 只能由client/server创建
	extern TestLogger g_log;
	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
		friend class TcpClient;
		friend class TcpServer;

	public:
		void Close();
		bool Write(const char *data, size_t trans);
		void StartRead(); // start read loop

		TcpConn(io_ctx& ctx);		 // for client
		TcpConn(TcpSock &&sock); // for server
		~TcpConn();
	protected:
		void read_head_handler(const NetErr&, size_t);
		void read_body_handler(const NetErr &, size_t);
		void write_handler(const NetErr &, size_t);
		void err_handler(const NetErr &);	// 关闭socket，错误输出
		// void (*net_proc)(const char *data, size_t trans);

	private:
		TcpSock sock_;
		std::mutex sendLock;
		BlockSendBuffer<DEFAULT_SEND_BUFFER_SIZE, DEFAULT_SEND_BUFFER_POOL_EXTEND_SIZE> sendBuffer;	// if no send,0KB
		
		char readBuffer[AN_MSG_MAX_SIZE];	// 64KB
		TestLogger* logger = new TestLogger;
		// std::shared_ptr<ILogger> logger;
	};

	// NewTCPConn()
	// NewUDPConn()
	// NewKCPConn()
	// class TcpConnFactory {
	//
	//};

}
