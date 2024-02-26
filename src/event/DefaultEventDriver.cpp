#include "DefaultEventDriver.h"
#include "../utils.h"

namespace AsioNet
{
	DefaultEventDriver::DefaultEventDriver()
	{
	}

	DefaultEventDriver::~DefaultEventDriver()
	{
		if (ptr_accept)		delete ptr_accept;
		if (ptr_connect)	delete ptr_connect;
		if (ptr_disconnect)	delete ptr_disconnect;
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
	void DefaultEventDriver::PushRecv(NetKey k,const char* data, size_t trans)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
		});
		m_recvBuffer.Push(data,trans);
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
				// 1.
				// 效率最高的方式，减少拷贝
				char* data = nullptr;
				size_t len = m_recvBuffer.PopUnsafe(&data);
				if (len < 4)
				{
					break;
				}
				
				// unpack，这里实际以哪种方式需要和服务端协商
				uint16_t msgID = *((uint16_t*)data);
				uint16_t flag = *((uint16_t*)(data+2));

				auto itr = m_routers.find(msgID);
				if (itr != m_routers.end())
				{
					// *use one object*
					Router& router = itr->second;
					router.pb->Clear();

					if(!router.pb->ParseFromArray(data + 4, len - 4)) {
						/*warning*/
						break;
					}

					router.h->Handle(*(router.pb));
					/*	how to use
					{
						// 这边缺少类型检查
						1.const MyPb& pb = dynamic_cast<const MyPb&>(proto);	// 只有这个是安全的
						2.const MyPb& pb = static_cast<const MyPb&>(proto);
						3.const MyPb& pb = (const MyPb&)(proto);
					}
					*/
				}

				break;
			}
			case EventType::Accept:	// 如果用对象的方式，C++如何绕过对象访问权限的问题
			{
				if (ptr_accept)
				{
					auto addr = NetKey2Addr(e.key);
					ptr_accept->Handle(e.key, addr.ip, addr.port);
				}
				break;
			}
			case EventType::Connect:
			{
				if (ptr_connect)
				{
					auto addr = NetKey2Addr(e.key);
					ptr_connect->Handle(e.key, addr.ip, addr.port);
				}
				break;
			}
			case EventType::Disconnect:
			{
				if (ptr_connect)
				{
					auto addr = NetKey2Addr(e.key);
					ptr_disconnect->Handle(e.key, addr.ip, addr.port);
				}
				break;
			}
		}
		m_events.pop();
		return true;
	}
}