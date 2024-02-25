#pragma once

#include <ikcp.h>

#include "../include/AsioNetDef.h"
#include "../include/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	struct IKcpConnOwner;	// 前向声明
	// doc:https://github.com/libinzhangyuan/asio_kcp

	// ikcp_allocator:考虑接管内存管理，默认的方式太粗暴

	class KcpConn : public std::enable_shared_from_this<KcpConn>
	{
	public:
		KcpConn() = delete;
		KcpConn(const KcpConn&) = delete;
		KcpConn(KcpConn&&) = delete;
		KcpConn& operator=(const KcpConn&) = delete;
		KcpConn& operator=(KcpConn&&) = delete;

		KcpConn(io_ctx& ctx, IEventPoller* p);

		~KcpConn();

		void SetOwner(IKcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		void Close();

		NetKey Key();
		
		// 只能在connect/accept成功了之后使用
		void StartRead(); 

	protected:
	    static int kcpOutPutFunc(const char *buf, int len,ikcpcb *kcp, void *user);

		void init();

		void kcpUpdateFunc();

		// void write_handler(const NetErr&, size_t);

		void err_handler();

	private:
        // bind 会报错
        // 依然需要先connect才能发送成功
		UdpSock m_sock;
        ikcpcb *m_kcp = nullptr;
		std::mutex m_kcpLock;

		// BlockSendBuffer<1024,2> m_sendBuffer;
        // 用于接受kcp协议的buffer，kcp协议经过分片处理，不需要很大，这里看4096比较顺眼就用这个数字了
		char m_kcpBuffer[AN_KCP_BUFFER_SIZE];
		char m_readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;

		IEventPoller* ptr_poller;
		IKcpConnOwner* ptr_owner;
	};

	struct IKcpConnOwner {	// 为了管理连接而设计
		virtual void AddConn(std::shared_ptr<KcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~IKcpConnOwner() {}
	};
}
