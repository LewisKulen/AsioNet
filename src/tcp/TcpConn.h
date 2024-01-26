#pragma once

#include "../include/AsioNetDef.h"
#include "../include/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	struct ITcpConnOwner;	// 前向声明

	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	constexpr unsigned int SEND_BUFFER_SIZE = 8 * 1024;// AN_MSG_MAX_SIZE;
	// constexpr unsigned int SEND_BUFFER_SIZE = AN_MSG_MAX_SIZE;// AN_MSG_MAX_SIZE;
	const unsigned int SEND_BUFFER_EXTEND_NUM = 2;
	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
	public:
		TcpConn() = delete;
		TcpConn(const TcpConn&) = delete;
		TcpConn(TcpConn&&) = delete;
		TcpConn& operator=(const TcpConn&) = delete;
		TcpConn& operator=(TcpConn&&) = delete;

		// 使用说明：请配合shared_ptr使用，直接在栈上使用即可
		// auto conn = std::make_shared<TcpConn>(ctx, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// conn->Connect(... ...);	
		// 异步连接成功之后，会自动调用owner->AddConn
		// 请不要再没有连接成功时，自己调用owner->AddConn,逻辑上也不正确
		TcpConn(io_ctx& ctx, IEventPoller* p);

		// TcpServer在accept时使用
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// owner->AddConn(conn);	// 这里因为连接已经成功了，直接在Conn外部Add是合理的
		TcpConn(TcpSock&& sock, IEventPoller* p);
		
		~TcpConn();

		// 这个操作不是必须得
		// 例如：如果不需要管理连接，且全是无状态服务，这个操作就完全不需要,只需如下两步即可
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->StartRead()
		void SetOwner(ITcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		void Connect(std::string addr, unsigned short port, int retry);

		// 主动关闭这个连接，一旦关闭连接，就会调用owner->DelConn
		// owner不应该再继续拥有conn的所有权，因为底层的sock已经关闭，接下来的所有操作都将失败
		// 只会调用一次
		void Close();

		// 只能在connect/accept成功了之后使用
		void StartRead(); 
		
		NetKey Key();
	protected:
		void init();

		void read_handler(const NetErr&, size_t);
		void write_handler(const NetErr&, size_t);

		void err_handler(const NetErr&);

	private:
		TcpSock m_sock;
		std::mutex m_sendLock;
		// 缺点：缓冲区大小修改起来得重新编译
		BlockSendBuffer<SEND_BUFFER_SIZE,
			SEND_BUFFER_EXTEND_NUM> m_sendBuffer;
		char m_readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;
		bool m_close;
		std::mutex m_closeLock;
		IEventPoller* ptr_poller;
		ITcpConnOwner* ptr_owner;
	};

	struct ITcpConnOwner {	// 为了管理连接而设计
		virtual void AddConn(std::shared_ptr<TcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~ITcpConnOwner() {}
	};
}
