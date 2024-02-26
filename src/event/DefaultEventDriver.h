#pragma once

#include "IEventPoller.h"

#include "../include/BlockBuffer.h"
#include "../include/AsioNetDef.h"

#include <queue>
#include <map>

#include <google/protobuf/message_lite.h>

namespace AsioNet
{
	using GooglePbLite = google::protobuf::MessageLite;

	class IHandler {
	public:
		virtual void Handle(const GooglePbLite&) = 0;
		virtual ~IHandler() {}
	};

	class IAcceptHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IAcceptHandler() {}
	};
	class IConnectHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IConnectHandler() {}
	};
	class IDisconnectHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IDisconnectHandler() {}
	};


	// --------------------------------------------------------------------------------
	// EventDriver和业务逻辑应该是强关联的，不同的业务逻辑适配不同的运行架构，才能发挥更高的性能
	const unsigned int DEFAULT_POLLER_BUFFER_SIZE = AN_MSG_MAX_SIZE;
	const unsigned int DEFAULT_POLLER_BUFFER_EXTEND_NUM = 2;
	class DefaultEventDriver : public IEventPoller
	{
		// 内部自用的数据结构
		enum class EventType
		{
			Accept,
			Connect,
			Disconnect,
			Recv,
			Error,
		};
		struct NetEvent
		{
			NetKey key;
			EventType type;
		};
		struct Router {
			Router(GooglePbLite* a1, IHandler* a2) :
				pb(a1), h(a2) {}

			~Router()
			{
				if (pb)	delete pb;
				if (h)	delete h;
			}

			GooglePbLite* pb;
			IHandler* h;
		};

	public:
		DefaultEventDriver();
		~DefaultEventDriver();

		// 衔接底层的接口
		void PushAccept(NetKey k) override;
		void PushConnect(NetKey k) override;
		void PushDisconnect(NetKey k) override;
		void PushRecv(NetKey k, const char *data, size_t trans) override;

		// *********************** 实际对外的方法 **************************
		// 取出一个Event并交给特定的处理器
		bool PopOne();

		template<typename PB, typename H>
		void AddRouter(uint16_t msgID)
		{
			PB* pb = new PB;
			H* h = new H;

			static_assert(dynamic_cast<GooglePbLite*>(pb) != nullptr);
			static_assert(dynamic_cast<IHandler*>(h) != nullptr);

			m_routers[msgID] = Router(pb, h);
		}

		template<typename H>
		void SetAcceptHandler()
		{
			H* h = new H;
			static_assert(dynamic_cast<IAcceptHandler*>(h) != nullptr);
			ptr_accept = h;
		}

		template<typename H>
		void SetConnectHandler()
		{
			H* h = new H;
			static_assert(dynamic_cast<IConnectHandler*>(h) != nullptr);
			ptr_connect = h;
		}

		template<typename H>
		void SetDisconnectHandler()
		{
			H* h = new H;
			static_assert(dynamic_cast<IDisconnectHandler*>(h) != nullptr);
			ptr_disconnect = h;
		}
		// ************************************************************

	protected:
	private:
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		BlockBuffer<DEFAULT_POLLER_BUFFER_SIZE,
					DEFAULT_POLLER_BUFFER_EXTEND_NUM> m_recvBuffer;

		std::map<uint16_t, Router> m_routers;

		IAcceptHandler* ptr_accept;
		IConnectHandler* ptr_connect;
		IDisconnectHandler* ptr_disconnect;
	};
}