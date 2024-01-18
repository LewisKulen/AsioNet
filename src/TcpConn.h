#pragma once

#include "AsioNetDef.h"
#include "BlockBuffer.h"
#include "Event.h"

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	constexpr unsigned int SEND_BUFFER_SIZE = 1024 * 8;
	const unsigned int SEND_BUFFER_EXTEND_NUM = 2;

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
		// 发送缓冲可以考虑做的大一点
		BlockSendBuffer<SEND_BUFFER_SIZE,
						SEND_BUFFER_EXTEND_NUM> sendBuffer;
		char readBuffer[AN_MSG_MAX_SIZE];
		NetKey key_;
		IEventPoller* poller;
	};

	struct ITcpConnFactory{
		virtual std::shared_ptr<TcpConn> NewConn() = 0;
		virtual ~ITcpConnFactory(){}
	};

}
