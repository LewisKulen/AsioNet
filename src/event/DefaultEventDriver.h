#pragma once

#include "IEventPoller.h"

#include "../include/BlockBuffer.h"
#include "../include/AsioNetDef.h"

#include <queue>
#include <unordered_map>

#include <google/protobuf/message_lite.h>

namespace AsioNet
{
	using GooglePbLite = google::protobuf::MessageLite;

	class IHandler {
	public:
		virtual void Handle(const GooglePbLite&) = 0;
		virtual ~IHandler() {}
	};

	class IAcceptHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IAcceptHandler() {}
	};
	class IConnectHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IConnectHandler() {}
	};
	class IDisconnectHandler {
	public:
		virtual void Handle(NetKey nk, const std::string ip, uint16_t port) = 0;
		virtual ~IDisconnectHandler() {}
	};


	// --------------------------------------------------------------------------------
	// EventDriver和业务逻辑应该是强关联的，不同的业务逻辑适配不同的运行架构，才能发挥更高的性能
	const unsigned int DEFAULT_POLLER_BUFFER_SIZE = AN_MSG_MAX_SIZE;
	const unsigned int DEFAULT_POLLER_BUFFER_EXTEND_NUM = 2;
	class DefaultEventDriver : public IEventPoller
	{
		// 内部自用的数据结构
		enum class EventType
		{
			Accept,
			Connect,
			Disconnect,
			Recv,
			Error,
		};
		struct NetEvent
		{
			NetKey key;
			EventType type;
		};
	public:
		DefaultEventDriver();
		~DefaultEventDriver();

		// 衔接底层的接口
		void PushAccept(NetKey k) override;
		void PushConnect(NetKey k) override;
		void PushDisconnect(NetKey k) override;
		void PushRecv(NetKey k, const char* data, size_t trans) override;

		// *********************** 实际对外的方法 **************************
		// 取出一个Event并交给特定的处理器
		bool PopOne();

		/*
		HANDLER =
		class DemoHandler{
			void operator()(NetKey ne,const PB& pb)
			{
				...
			}
		};
		*/
		template<typename HANDLER, typename PB>
		void AddRouter(uint16_t msgID)
		{
			// 后续可以加点模板参数检查
			m_routers[msgID] = std::function(
				[](NetKey key, uint16_t flag, const char* data, uint16_t len)->void {
					// 这里请不要越界，因为默认采用的是PopUnsafe
					// 这里是对所有协议的统一处理的地方，请谨慎修改哦！
					PB pb;
					if (!pb.ParseFromArray(data, len))
					{
						// err handle
						return;
					}

					HANDLER{}(key, pb);
				});
		}

		
		// ************************************************************

	protected:
	private:
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		BlockBuffer<DEFAULT_POLLER_BUFFER_SIZE,
			DEFAULT_POLLER_BUFFER_EXTEND_NUM> m_recvBuffer;

		std::unordered_map<uint16_t,
			std::function<void(NetKey key, uint16_t, const char*, uint16_t)>> m_routers;

	
		std::vector<std::function<void(NetKey,std::string,uint16_t)>> m_handler;
		std::function <void(NetKey)> m_errHandler;
	};
}