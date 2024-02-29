#include "DefaultEventDriver.h"
#include "../utils.h"

namespace AsioNet
{
	DefaultEventDriver::DefaultEventDriver()
	{
	}

	DefaultEventDriver::~DefaultEventDriver()
	{

	}

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
	void DefaultEventDriver::PushRecv(NetKey k, const char* data, size_t trans)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
			});
		m_recvBuffer.Push(data, trans);
	}

	bool DefaultEventDriver::PopOne()
	{
		_lock_guard_(m_lock);
		if (m_events.empty()) {
			return false;
		}

		auto e = m_events.front();
		switch (e.type) {
		case EventType::Recv:
		{
			// 默认效率最高的方式，减少拷贝
			char* data = nullptr;
			size_t len = m_recvBuffer.PopUnsafe(&data);
			if (len < 4)
			{
				// err handle
				break;
			}

			// unpack，这里实际以哪种方式需要和服务端协商
			uint16_t msgID = *((uint16_t*)data);
			uint16_t flag = *((uint16_t*)(data + 2));
			
			auto itr = m_routers.find(msgID);
			if (itr != m_routers.end())
			{
				auto& f = itr->second;
				f(e.key, flag, data + 4, len - 4);
			}
			else
			{
				// err handle
			}

			break;
		}
		case EventType::Accept:	// 如果用对象的方式，C++如何绕过对象访问权限的问题
		{
			break;
		}
		case EventType::Connect:
		{
			break;
		}
		case EventType::Disconnect:
		{
			break;
		}
		}
		m_events.pop();
		return true;
	}
}