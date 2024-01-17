#include "Event.h"

namespace AsioNet
{
	void DefaultEventPoller::PushAccept(const NetKey& k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept
		});
	}
	void DefaultEventPoller::PushConnect(const NetKey& k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect
		});
	}
	void DefaultEventPoller::PushDisconnect(const NetKey& k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Disconnect
		});
	}
	void DefaultEventPoller::PushRecv(const NetKey& k,const char* data, size_t trans)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
		});
		
	}

	



	DefaultEventPoller::DefaultEventPoller()
	{
		SetHandler(DefaultEventHandler::GetInstance());
	}
	void DefaultEventPoller::SetHandler(IEventHandler* h)
	{
		m_handler = h;
	}
	DefaultEventPoller* DefaultEventPoller::GetInstance()
	{
		static DefaultEventPoller m_poller;
		return &m_poller;
	}

}