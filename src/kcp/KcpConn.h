#pragma once

#include "ikcp.h"

#include "../include/AsioNetDef.h"
#include "../include/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	struct IKcpConnOwner;	// 前向声明

	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html

	class KcpConn : public std::enable_shared_from_this<KcpConn>
	{
	public:
		KcpConn() = delete;
		KcpConn(const KcpConn&) = delete;
		KcpConn(KcpConn&&) = delete;
		KcpConn& operator=(const KcpConn&) = delete;
		KcpConn& operator=(KcpConn&&) = delete;

		// 使用说明：请配合shared_ptr使用，直接在栈上使用即可
		// auto conn = std::make_shared<TcpConn>(ctx, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// conn->Connect(... ...);	
		// 异步连接成功之后，会自动调用owner->AddConn
		// 请不要再没有连接成功时，自己调用owner->AddConn,逻辑上也不正确
		KcpConn(io_ctx& ctx, IEventPoller* p);

		~KcpConn();

		// 这个操作不是必须得
		// 例如：如果不需要管理连接，且全是无状态服务，这个操作就完全不需要,只需如下两步即可
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->StartRead()
		void SetOwner(IKcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		// 主动关闭这个连接，一旦关闭连接，就会调用owner->DelConn
		// owner不应该再继续拥有conn的所有权，因为底层的sock已经关闭，接下来的所有操作都将失败
		// 只会调用一次
		void Close();

		NetKey Key();
	protected:
		void init();

        // 只能在connect/accept成功了之后使用
		void ReadLoop(); 

		void write_handler(const NetErr&, size_t);

		void err_handler(const NetErr&);

	private:
        // bind 会报错
        // 依然需要先connect才能发送成功
		UdpSock m_sock;
        ikcpcb *m_kcp = nullptr;
		std::mutex m_sendLock;
		// 缺点：缓冲区大小修改起来得重新编译
		BlockSendBuffer<1024,2> m_sendBuffer;
        char m_kcpBuffer[AN_MSG_MAX_SIZE];
		char m_readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;
		bool m_close;
		std::mutex m_closeLock;
		IEventPoller* ptr_poller;
		IKcpConnOwner* ptr_owner;
	};

	struct IKcpConnOwner {	// 为了管理连接而设计
		virtual void AddConn(std::shared_ptr<KcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~IKcpConnOwner() {}
	};
}
