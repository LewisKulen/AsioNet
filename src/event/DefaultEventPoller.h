#pragma once

#include "IEventPoller.h"
#include "IEventHandler.h"

#include "../include/BlockBuffer.h"
#include "../include/AsioNetDef.h"

#include <queue>

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
	
	const unsigned int DEFAULT_POLLER_BUFFER_SIZE = AN_MSG_MAX_SIZE;
	const unsigned int DEFAULT_POLLER_BUFFER_EXTEND_NUM = 2;
	class DefaultEventPoller : public IEventPoller
	{
	public:
		DefaultEventPoller(IEventHandler*);
		~DefaultEventPoller();

		void PushAccept(NetKey k) override;
		void PushConnect(NetKey k) override;
		void PushDisconnect(NetKey k) override;
		void PushRecv(NetKey k, const char *data, size_t trans) override;
		void PushError(NetKey k,const NetErr&);

		bool PopOne();

	protected:
	private:
		IEventHandler *ptr_handler;
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		std::queue<NetErr> m_errs;
		BlockBuffer<DEFAULT_POLLER_BUFFER_SIZE,
					DEFAULT_POLLER_BUFFER_EXTEND_NUM> m_recvBuffer;
	};
}