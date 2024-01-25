#include "DefaultEventDriver.h"

namespace AsioNet
{
	DefaultEventDriver::DefaultEventDriver(IEventHandler* h)
	{
		ptr_handler = h;
		memset(m_tempBuffer,0,sizeof(m_tempBuffer));
	}
	DefaultEventDriver::~DefaultEventDriver()
	{}

	void DefaultEventDriver::PushAccept(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept
		});
	}
	void DefaultEventDriver::PushConnect(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect
		});
	}
	void DefaultEventDriver::PushDisconnect(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Disconnect
		});
	}
	void DefaultEventDriver::PushRecv(NetKey k,const char* data, size_t trans)
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
	void DefaultEventDriver::PushError(NetKey k, const NetErr& err)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Error
			});
		m_errs.push(err);
	}
	bool DefaultEventDriver::PopOne()
	{
		_lock_guard_(m_lock);
		if (m_events.empty()) {
			return false;
		}

		auto e = m_events.front();
		switch(e.type){
			case EventType::Recv:
			{
				// 内部以效率最高的方式来，减少拷贝
				char* d = nullptr;
				size_t len = m_recvBuffer.PopUnsafe(&d);
				// 1.find router
				// 2.handler
				ptr_handler->RecvHandler(e.key,d,len);
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
			case EventType::Error:
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