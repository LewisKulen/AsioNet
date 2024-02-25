#pragma once

#include "IEventPoller.h"
#include "IEventHandler.h"

#include "../include/BlockBuffer.h"
#include "../include/AsioNetDef.h"

#include <queue>
#include <map>

#include <google/protobuf/message_lite.h>

namespace AsioNet
{
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

	using GooglePbLite = google::protobuf::MessageLite;

	class IHandler {
	public:
		virtual void Handle(const GooglePbLite*) = 0;
		~IHandler() {}
	};


	const unsigned int DEFAULT_POLLER_BUFFER_SIZE = AN_MSG_MAX_SIZE;
	const unsigned int DEFAULT_POLLER_BUFFER_EXTEND_NUM = 2;
	class DefaultEventDriver : public IEventPoller
	{
	public:
		DefaultEventDriver(IEventHandler*);
		~DefaultEventDriver();

		void PushAccept(NetKey k) override;
		void PushConnect(NetKey k) override;
		void PushDisconnect(NetKey k) override;
		void PushRecv(NetKey k, const char *data, size_t trans) override;

		bool PopOne();

		struct Router {
			template<typename PB, typename H>
			Router() 
			{
				pb = new PB;
				h = new H;
			}
			~Router()
			{
				if (pb)	delete pb;
				if (h)	delete h;
			}
			
			GooglePbLite* pb;
			IHandler* h;
		};
		template<typename PB, typename H>
		void AddRouter(unsigned short msgID)
		{
			m_routers[msgID] = Router<PB,H>()
		}

	protected:
	private:
		IEventHandler *ptr_handler;
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		std::queue<NetErrCode> m_errs;
		BlockBuffer<DEFAULT_POLLER_BUFFER_SIZE,
					DEFAULT_POLLER_BUFFER_EXTEND_NUM> m_recvBuffer;

		std::map<unsigned short, Router> m_routers;

	};
}