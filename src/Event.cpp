#include "Event.h"

namespace AsioNet
{
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