#pragma once

#include "AsioNetDef.h"
#include "BlockBuffer.h"
#include "Event.h"

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	// 只能由client/server创建

	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
		// 紧耦合，friend
		friend class TcpClient;
		friend class TcpServer;

	public:
		void Close();
		bool Write(const char *data, size_t trans);
		void StartRead(); // start read loop

		TcpConn(io_ctx& ctx);		 // for client
		TcpConn(TcpSock &&sock); // for server
		NetKey GetKey();
		~TcpConn();
	protected:
		void init();

		void read_handler(const NetErr &, size_t);
		void write_handler(const NetErr &, size_t);

		void err_handler(const NetErr &);

	private:
		TcpSock sock_;
		std::mutex sendLock;
		BlockSendBuffer<DEFAULT_SEND_BUFFER_SIZE, DEFAULT_SEND_BUFFER_POOL_EXTEND_SIZE> sendBuffer;	// if no send,0KB
		char readBuffer[AN_MSG_MAX_SIZE];
		IEventPoller* poller;
	};

	class TcpConnFactory {
	public:
		std::shared_ptr<TcpConn> NewSeverClient();
		std::shared_ptr<TcpConn> NewClient();
		TcpConnFactory* GetInstance();
	private:
		std::map<NetKey, std::shared_ptr<TcpConn>> m_conns;
	};

}
