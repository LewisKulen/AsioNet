#pragma once

#include "IEventPoller.h"

#include "../utils/BlockBuffer.h"
#include "../utils/AsioNetDef.h"

#include <google/protobuf/message_lite.h>

#include <queue>
#include <unordered_map>
#include <type_traits>
#include <functional>

namespace AsioNet
{
	enum class EventErrCode
	{
		SUCCESS,
		RECV_ERR,
		UNKNOWN_MSG_ID,
		PRASE_PB_ERR,
	};

	template<class HANDLER, class ...Args>
	constexpr bool check_functor_v =
		std::is_object_v<HANDLER> && std::is_invocable_v<HANDLER, Args...>;

	template<class HANDLER, class ...Args>
	constexpr bool check_function_v =
		std::is_function_v<HANDLER> && std::is_invocable_v<HANDLER, Args...>;

	using GooglePbLite = google::protobuf::MessageLite;

	// EventDriver和业务逻辑应该是强关联的
	class EventDriver final: public IEventPoller
	{
		// 内部使用类
		enum class EventType
		{
			Accept = 0,
			Connect,
			Disconnect,
			Recv,
			Error,
		};
		
		struct NetEvent
		{
			NetKey key;
			EventType type;
			std::string ip;
			uint16_t port;
		};

		class Package {
		public:
			Package()
			{
				memset(this, 0, sizeof(Package));
			}
			bool Unpack(char* bytes, size_t trans)
			{
				if (trans < 4) {
					return false;
				}
				msgid = *((uint16_t*)(bytes));
				flag = *((uint16_t*)(bytes + 2));
				data = bytes + 4;
				datalen = trans - 4;
				return true;
			}
			uint16_t GetMsgID() const { return msgid; }
			uint16_t GetFlag() const { return flag; }
			const char* GetData() const { return data; }
			size_t GetDataLen() const { return datalen; }
		private:
			uint16_t msgid, flag;
			char* data;
			size_t datalen;
		};

		template<typename HANDLER, typename PB>
		EventErrCode wrapped_event_handler(void* user,NetKey key, const Package& pkg)
		{
			// 模板参数检查
			static_assert(std::is_base_of_v<GooglePbLite, PB>, "not a protobuf");
			static_assert(check_functor_v<HANDLER, void*,NetKey, const PB&>,
				"functor need && token is: void(NetKey,const GooglePbLite&)");

			// 对所有协议都需要的操作请在这里操作
			PB pb;
			if (!pb.ParseFromArray(pkg.GetData(), pkg.GetDataLen()))
			{
				return EventErrCode::PRASE_PB_ERR;
			}

			// must be a functor
			HANDLER{}(user, key, pb);
			return EventErrCode::SUCCESS;
		}

	public:
		EventDriver();
		~EventDriver() override;

		// 衔接底层的接口
		void PushAccept(NetKey k, const std::string& ip, uint16_t port) override;
		void PushConnect(NetKey k, const std::string& ip, uint16_t port) override;
		void PushDisconnect(NetKey k, const std::string& ip, uint16_t port) override;
		void PushRecv(NetKey k, const char* data, size_t trans) override;

		// 取出一个Event并交给特定的处理器
		bool RunOne();

		/*
		// 1.必须有默认构造函数
		// 2.函数签名如下
		// 3.必须是仿函数
		class DemoHandler{
		public:
			void operator()(NetKey ne,const PB& pb){}
		};
		*/
		template<typename HANDLER, typename PB>
		void AddRouter(void* user, uint16_t msgID)
		{
			EventCaller caller;
			caller.func = &EventDriver::wrapped_event_handler<HANDLER,PB>;
			caller.user = user;
			m_routers[msgID] = caller;
			// 你也可以使用lambda + function实现该功能
			// 我这里为了效率，决定自己实现一个极简版的
		}

		template<typename HANDLER>
		void RegisterAcceptHandler(void* user)
		{
			static_assert(check_functor_v<HANDLER, void*, NetKey, std::string, uint16_t>,
				"RegisterAcceptHandler,functor need && token is: void(NetKey, std::string, uint16_t)");
			m_handler[static_cast<int>(EventType::Accept)] = std::function(
				[user](NetKey key, std::string ip, uint16_t port)->void {
					HANDLER{}(user,key, ip, port);
				});
		}

		template<typename HANDLER>
		void RegisterConnectHandler(void* user)
		{
			static_assert(check_functor_v<HANDLER, void*, NetKey, std::string, uint16_t>,
				"RegisterConnectHandler,functor need && token is: void(NetKey, std::string, uint16_t)");
			m_handler[static_cast<int>(EventType::Connect)] = std::function(
				[user](NetKey key, std::string ip, uint16_t port)->void {
					HANDLER{}(user,key, ip, port);
				});
		}

		template<typename HANDLER>
		void RegisterDisconnectHandler(void* user)
		{
			static_assert(check_functor_v<HANDLER, void*, NetKey, std::string, uint16_t>,
				"RegisterDisconnectHandler,functor need && token is: void(NetKey, std::string, uint16_t)");
			m_handler[static_cast<int>(EventType::Disconnect)] = std::function(
				[user](NetKey key, std::string ip, uint16_t port)->void {
					HANDLER{}(user,key, ip, port);
				});
		}

		template<typename HANDLER>
		void RegisterErrHandler(void* user)
		{
			static_assert(check_functor_v<HANDLER, void* ,NetKey, EventErrCode>,
				"RegisterErrHandler,functor need && token is: void(NetKey)");
			m_errHandler = std::function(
				[user](NetKey key, EventErrCode ec)->void {
					HANDLER{}(user,key, ec);
				});
		}

	private:
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		BlockBuffer<AN_MSG_MAX_SIZE, 2> m_recvBuffer;

		struct EventCaller{
			EventCaller():func(nullptr),user(nullptr){}
			EventErrCode(EventDriver::*func)(void* user,NetKey, const Package&);
			void* user;
		};
		std::unordered_map<uint16_t,EventCaller> m_routers;

		std::function<void(NetKey, std::string, uint16_t)> m_handler[3];
		std::function <void(NetKey, EventErrCode)> m_errHandler;
	};
}