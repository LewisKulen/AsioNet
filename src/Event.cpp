#include "Event.h"

namespace AsioNet
{
	void DefaultEventPoller::PushAccept(NetKey k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept
		});
	}
	void DefaultEventPoller::PushConnect(NetKey k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect
		});
	}
	void DefaultEventPoller::PushDisconnect(NetKey k)
	{
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Disconnect
		});
	}
	void DefaultEventPoller::PushRecv(NetKey k,const char* data, size_t trans)
	{
		// 同一个库在Send的时候保证trans>0才行，
		// 如果是别的库Send的数据长度是0，这里校验一下
		// 这里需要写个例程验证下，如果发0，会出现什么
		if(trans <= 0){
			return;
		}
		_lock_guard_ g(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
		});
		m_recvBuffer.Push(data,trans);
	}

	DefaultEventPoller::DefaultEventPoller(IEventHandler* h)
	{
		ptr_handler = h;
	}
}