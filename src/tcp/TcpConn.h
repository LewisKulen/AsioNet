#pragma once

#include "../utils/AsioNetDef.h"
#include "../utils/BlockBuffer.h"
#include "../event/IEventPoller.h"

#include <unordered_map>

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html

	using TcpSock = asio::ip::tcp::socket;
	using TcpEndPoint = asio::ip::tcp::endpoint;

	struct ITcpConnOwner;	// 前向声明

	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
	public:
		TcpConn() = delete;
		TcpConn(const TcpConn&) = delete;
		TcpConn(TcpConn&&) = delete;
		TcpConn& operator=(const TcpConn&) = delete;
		TcpConn& operator=(TcpConn&&) = delete;

		// 请使用shared_ptr来创建conn，直接在栈上使用即可
		// 实例：
		// auto conn = std::make_shared<TcpConn>(ctx, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// conn->Connect(... ...);	
		TcpConn(io_ctx& ctx, IEventPoller* p);

		// TcpServer在accept时使用
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// owner->AddConn(conn);	
		// 这里因为连接实在Conn外实现的，所以不会调用AddConn，外部自己管理即可
		TcpConn(TcpSock&& sock, IEventPoller* p);
		
		~TcpConn();

		void SetOwner(ITcpConnOwner*);
		
		// 发送数据
		bool Write(const char* data, size_t trans);

		// 开始异步连接
		// 成功之后调用ptr_poller->PushConnect
		void Connect(const std::string& ip, uint16_t port, int retry/*失败重试次数*/);

		// 主动关闭这个连接，只会调用一次
		// 调用ptr_owner->DelConn，ps：owner不应该再继续拥有conn的所有权，因为底层的sock已经关闭，接下来的所有操作都将失败
		// 调用ptr_poller->PushDisconnect
		void Close();

		// 开始接收数据
		// connect成功之后自动调用，accept的请手动调用
		// 成功接收数据后，调用ptr_poller->PushRecv
		void StartRead(); 
		
		// 获取对端addr
		TcpEndPoint Remote();

		// 唯一id
		NetKey Key();
	protected:
		void init();

		void read_handler(const NetErr&, size_t);
		void write_handler(const NetErr&, size_t);

		// 出错直接关闭连接
		void err_handler();

	private:
		TcpSock m_sock;

		std::mutex m_sendLock;
		// 发送一次数据的最大数据量
		static constexpr uint32_t SEND_BUFFER_SIZE = 8 * 1024;
		static const uint32_t SEND_BUFFER_EXTEND_NUM = 2;
		// 发送缓冲区
		BlockSendBuffer<SEND_BUFFER_SIZE,
			SEND_BUFFER_EXTEND_NUM> m_sendBuffer;

		// 接收缓冲区
		char m_readBuffer[AN_MSG_MAX_SIZE];

		NetKey m_key;
		bool m_close;
		std::mutex m_closeLock;
		IEventPoller* ptr_poller;
		ITcpConnOwner* ptr_owner;
	};

	// 由于accept,connect,disconnect都是异步的，为了方便管理链接，引入这个接口
	// connect成功的时候，调用AddConn
	// disconnect的时候，调用DelConn
	struct ITcpConnOwner {
		virtual void AddConn(std::shared_ptr<TcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~ITcpConnOwner() {}
	};

	// 内部用的一个ITcpConnOwner的实现
	class TcpConnMgr final:public ITcpConnOwner {
	public:
		void DelConn(NetKey) override;
		void AddConn(std::shared_ptr<TcpConn>) override;
		void Disconnect(NetKey k);

		std::shared_ptr<TcpConn> GetConn(NetKey);
		void Broadcast(const char*,size_t trans);

		~TcpConnMgr() override;
	private:
		std::unordered_map<NetKey,std::shared_ptr<TcpConn>> m_conns;
		std::mutex m_lock;
	};
}
