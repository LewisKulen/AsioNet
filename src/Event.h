#pragma once

#include "AsioNetDef.h"
#include <queue>

namespace AsioNet
{

	/*
	class TcpConn{
	private:
		IEventPoller*
	};
	*/

	struct IEventPoller {
		virtual void PushAccept(const TcpEndPoint& remote) = 0;
		virtual void PushConnect(const TcpEndPoint& remote) = 0;
		virtual void PushDisconnect(const TcpEndPoint& remote) = 0;
		virtual void PushRecv(const char* data, size_t trans) = 0;
		// void (*PushAccept)(const TcpEndPoint& remote);
		// void (*PushConnect)(const TcpEndPoint& remote);
		// void (*PushDisconnect)(const TcpEndPoint& remote);
		// void (*PushRecv)(const char* data, size_t trans);
	};

	// 性能优先
	// 内部不做接口，减少开销
	// 也不用仿函数，减少调用
	struct IEventPoller1 {
		static IEventPoller1* GetInstance()
		{
			static IEventPoller1 m_poller;
			return &m_poller;
		}
		void (*PushAccept)(const TcpEndPoint& remote);
		void (*PushConnect)(const TcpEndPoint& remote);
		void (*PushDisconnect)(const TcpEndPoint& remote);
		void (*PushRecv)(const char* data, size_t trans);
	private:
		IEventPoller1()
		{
			// how can i bind to DefaultEventPoller?
			// PushAccept = &(static_cast<IEventPoller*>(DefaultEventPoller::GetInstance())->PushAccept);

		}
		IEventPoller* m_poller;
	};


	struct IEventHandler {
		virtual void AcceptHandler(NetKey) = 0;
		virtual void ConnectHandler(NetKey) = 0;
		virtual void DisconnectHandler(NetKey) = 0;
		virtual void RecvHandler(NetKey, const char* data, size_t trans) = 0;
		virtual ~IEventHandler() {}
	};

	enum class EventType {
		Accept,
		Connect,
		Disconnect,
		Recv,
	};
	struct NetEvent{
		NetKey key;
		EventType type;
	};

	// singleton
	class DefaultEventPoller/*:public IEventPoller */{
	public:
		void PushAccept(const NetKey& k);
		void PushConnect(const NetKey& k);
		void PushDisconnect(const NetKey& k);
		void PushRecv(const NetKey& k,const char* data, size_t trans);
		void SetHandler(IEventHandler*);

		bool PopOne(NetEvent&);
		static DefaultEventPoller* GetInstance();
		~DefaultEventPoller() {};
	protected:
		DefaultEventPoller();
	private:
		IEventHandler* m_handler;
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		// BlockBuffer recv_data;
	};

	struct DefaultEventHandler :public IEventHandler {
		void AcceptHandler(NetKey, const std::string& ip, unsigned short port) {};
		void ConnectHandler(NetKey, const std::string& ip, unsigned short port) {};
		void DisconnectHandler(NetKey, const std::string& ip, unsigned short port) {};
		void RecvHandler(NetKey, const char* data, size_t trans) {};

		static DefaultEventHandler* GetInstance()
		{
			static DefaultEventHandler m_handler;
			return &m_handler;
		}
	protected:
		DefaultEventHandler() {};
	};

#define ASIO_NET_SET_EVENT_HANDLER(h)	\
					DefaultEventPoller::GetInstance()->SetHandler(h);



	// 使用者：实现IEventHandler

}