#include "./EventDriver.h"
#include "../utils/utils.h"

namespace AsioNet
{
	EventDriver::EventDriver()
	{
		m_handler[static_cast<int>(EventType::Accept)] = std::function(
			[](NetKey, std::string, uint16_t)->void {});
		m_handler[static_cast<int>(EventType::Connect)] = std::function(
			[](NetKey, std::string, uint16_t)->void {});
		m_handler[static_cast<int>(EventType::Disconnect)] = std::function(
			[](NetKey, std::string, uint16_t)->void {});
		m_errHandler = std::function(
			[](NetKey, EventErrCode)->void {});
	}

	EventDriver::~EventDriver()
	{
	}

	void EventDriver::PushAccept(NetKey k, const std::string& ip, uint16_t port)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept,ip,port
			});
	}
	void EventDriver::PushConnect(NetKey k, const std::string& ip, uint16_t port)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect,ip,port
			});
	}
	void EventDriver::PushDisconnect(NetKey k, const std::string& ip, uint16_t port)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Disconnect,ip,port
			});
	}
	void EventDriver::PushRecv(NetKey k, const char* data, size_t trans)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Recv
			});
		m_recvBuffer.Push(data, trans);
	}

	// 注意：这是单线程处理消息
	bool EventDriver::RunOne()
	{
		_lock_guard_(m_lock);
		if (m_events.empty()) {
			return false;
		}

		auto e = m_events.front();
		switch (e.type) {
		case EventType::Recv:
		{
			// 以效率最高的方式获取数据，减少拷贝
			auto [data, len] = m_recvBuffer.PopUnsafe();

			Package pkg;
			if (!pkg.Unpack(data, len)) {
				m_errHandler(e.key, EventErrCode::RECV_ERR);
				break;
			}

			auto itr = m_routers.find(pkg.GetMsgID());
			if (itr != m_routers.end())
			{
				auto& caller = itr->second;
				EventErrCode ec = caller.func(caller.user,e.key, pkg);
				if (ec != EventErrCode::SUCCESS)
				{
					m_errHandler(e.key, ec);
				}
			}
			else
			{
				m_errHandler(e.key, EventErrCode::UNKNOWN_MSG_ID);
			}

			break;
		}
		case EventType::Accept:
		{
			m_handler[static_cast<int>(EventType::Accept)](e.key, e.ip, e.port);
			break;
		}
		case EventType::Connect:
		{
			m_handler[static_cast<int>(EventType::Connect)](e.key, e.ip, e.port);
			break;
		}
		case EventType::Disconnect:
		{
			m_handler[static_cast<int>(EventType::Disconnect)](e.key, e.ip, e.port);
			break;
		}
		}
		m_events.pop();
		return true;
	}
}