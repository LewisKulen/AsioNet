#include "EventDriver.h"
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

	void EventDriver::PushAccept(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Accept
			});
	}
	void EventDriver::PushConnect(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Connect
			});
	}
	void EventDriver::PushDisconnect(NetKey k)
	{
		_lock_guard_(m_lock);
		m_events.push(NetEvent{
			k,EventType::Disconnect
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
			// 所以可以以效率最高的方式获取数据，减少拷贝
			auto [data, len] = m_recvBuffer.PopUnsafe();

			Package pkg;
			if (!pkg.Unpack(data, len)) {
				m_errHandler(e.key, EventErrCode::RECV_ERR);
				break;
			}

			auto itr = m_routers.find(pkg.GetMsgID());
			if (itr != m_routers.end())
			{
				auto& f = itr->second;
				f(e.key, pkg);
			}
			else
			{
				m_errHandler(e.key, EventErrCode::UNKNOWN_MSG_ID);
			}

			break;
		}
		case EventType::Accept:
		{
			auto [ip, port] = NetKey2Addr(e.key);
			m_handler[static_cast<int>(EventType::Accept)](e.key, ip, port);
			break;
		}
		case EventType::Connect:
		{
			auto [ip, port] = NetKey2Addr(e.key);
			m_handler[static_cast<int>(EventType::Connect)](e.key, ip, port);
			break;
		}
		case EventType::Disconnect:
		{
			auto [ip, port] = NetKey2Addr(e.key);
			m_handler[static_cast<int>(EventType::Disconnect)](e.key, ip, port);
			break;
		}
		}
		m_events.pop();
		return true;
	}
}