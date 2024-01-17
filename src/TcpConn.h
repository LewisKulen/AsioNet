#pragma once

#include "AsioNetDef.h"
#include "BlockBuffer.h"
#include "Event.h"
#include <map>

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	// 只能由client/server创建

	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
		// 紧耦合，friend
		friend class TcpServer;

	public:
		void Close();
		bool Write(const char *data, size_t trans);
		
		void StartRead(); // start read loop
		void Connect(std::string addr, unsigned short port);

		TcpConn(io_ctx& ctx);		 // for client
		TcpConn(TcpSock &&sock); // for server
		NetKey GetKey();
		~TcpConn();
	protected:
		void init();

		void read_handler(const NetErr &, size_t);
		void write_handler(const NetErr &, size_t);
		void connect_handler(const NetErr& ec);

		void err_handler(const NetErr &);

	private:
		TcpSock sock_;
		std::mutex sendLock;
		BlockSendBuffer<SEND_BUFFER_SIZE> sendBuffer;	// if no send,0KB
		char readBuffer[AN_MSG_MAX_SIZE];
		NetKey key_;
		IEventPoller* poller;
	};

	// 一个mgr，一个io_ctx
	// mgr管理自己的conn

	class TcpConnFactory{
	public:
		std::shared_ptr<TcpConn> NewConn();
	private:
		std::mutex connLock;
		std::map<NetKey, std::shared_ptr<TcpConn>> m_conns;
	};

}
