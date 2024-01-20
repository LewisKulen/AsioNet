#include "DefaultEventPoller.h"

namespace AsioNet
{
	DefaultEventPoller::DefaultEventPoller(IEventHandler* h)
	{
		ptr_handler = h;
	}
	DefaultEventPoller::~DefaultEventPoller()
	{}

	void DefaultEventPoller::PushAccept(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept
		});
	}
	void DefaultEventPoller::PushConnect(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect
		});
	}
	void DefaultEventPoller::PushDisconnect(NetKey k)
	{
		_lock_guard_(m_lock);
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
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
		});
		m_recvBuffer.Push(data,trans);
	}
	void DefaultEventPoller::PushError(NetKey k, const NetErr& err)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Error
			});
		m_errs.push(err);
	}
	bool DefaultEventPoller::PopOne()
	{
		_lock_guard_(m_lock);
		if (m_events.empty()) {
			return false;
		}

		auto e = m_events.front();
		switch(e.type){
			case EventType::Recv:
			{
				ptr_handler->RecvHandler(e.key,m_recvBuffer.PopToString());
				break;
			}
			case EventType::Accept:
			{
				ptr_handler->AcceptHandler(e.key);
				break;
			}
			case EventType::Connect:
			{
				ptr_handler->ConnectHandler(e.key);
				break;	
			}
			case EventType::Disconnect:
			{
				ptr_handler->DisconnectHandler(e.key);
				break;
			}
			case EventType::Error :
			{
				ptr_handler->ErrorHandler(e.key, m_errs.front());
				m_errs.pop();
				break;
			}
		}
		m_events.pop();
		return true;
	}

}