#pragma once

#include "../include/AsioNetDef.h"
#include "../include/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	class ITcpConnOwner;

	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	constexpr unsigned int SEND_BUFFER_SIZE = 8 * 1024;// AN_MSG_MAX_SIZE;
	const unsigned int SEND_BUFFER_EXTEND_NUM = 2;
	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
		// 紧耦合，friend
		friend class TcpServer;
	public:
		TcpConn() = delete;
		TcpConn(const TcpConn&) = delete;
		TcpConn& operator=(const TcpConn&) = delete;

		TcpConn(io_ctx& ctx,IEventPoller*);		 // for client
		TcpConn(TcpSock &&sock,IEventPoller*); 	// for server

		bool Write(const char *data, size_t trans);
		
		void StartRead(); // start read loop
		void Connect(std::string addr, unsigned short port);

		NetKey GetKey();
		~TcpConn();
	protected:
		void close();
		void init();

		void read_handler(const NetErr &, size_t);
		void write_handler(const NetErr &, size_t);
		void connect_handler(const NetErr& ec);

		void err_handler(const NetErr &);

	private:
		TcpSock sock_;
		std::mutex sendLock;
		// 缺点，缓冲区大小修改起来得重新编译
		BlockSendBuffer<SEND_BUFFER_SIZE,
						SEND_BUFFER_EXTEND_NUM> sendBuffer;
		char readBuffer[AN_MSG_MAX_SIZE];
		NetKey key_;
		bool isClose;
		std::mutex closeLock;
		IEventPoller* ptr_poller;
		ITcpConnOwner* ptr_owner;
	};

	struct ITcpConnOwner{
		virtual void AddConn(std::shared_ptr<TcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~ITcpConnOwner(){}
	};
}
