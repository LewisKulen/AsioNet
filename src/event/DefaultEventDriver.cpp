#include "DefaultEventDriver.h"

namespace AsioNet
{
	DefaultEventDriver::DefaultEventDriver(IEventHandler* h)
	{
		ptr_handler = h;
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
				char* d = nullptr;
				size_t len = m_recvBuffer.PopUnsafe(&d);
				// assert(len != 0);
				// assert(d != nullptr);
				if (len < 4)
				{
					return;
				}
				
				unsigned short msgID = 1;
				unsigned short flag = 2;

				auto itr = m_routers.find(msgID);
				if (itr != m_routers.end())
				{
					// *use one object*
					Router& router = itr->second;
					// router.pb->ParseFromArray();
					router.h->Handle(router.pb);
					/*	how to use
					{
						const MyPb* pb = dynamic_cast<XXX>(proto);
					}
					*/
					// ptr_handler->RecvHandler(e.key, d, len);

				}


				// 2.
				// ptr_handler->RecvHandler(e.key,m_recvBuffer.PopToString());
				// 3.
				// size_t len = m_recvBuffer.Pop(m_recvBuffer);
				// ptr_handler->RecvHandler(e.key,m_recvBuffer,len);


				break;
			}
			case EventType::Accept:	// 如果用对象的方式，C++如何绕过对象访问权限的问题
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
		}
		m_events.pop();
		return true;
	}
}

void func()
{

}