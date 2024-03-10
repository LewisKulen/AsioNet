#pragma once

#include "../../src/AsioNet.h"

#include "../protoc/cpp_all_pb.h"

#include <type_traits>

struct Header {
	uint16_t msgid;
	uint16_t flag;
};

struct TestClient{
public:
	TestClient() :netMgr(2,&m_ed), m_conn(0)
	{
		InitRouter();
		std::thread([this]() {
			while (true) {
				while (m_ed.RunOne());
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			}).detach();

		memset(m_buffer, 0, sizeof(m_buffer));
		netMgr.Connect("127.0.0.1", 8888, 100);
	}

	template<typename PB>
	bool SendMsg(uint16_t msgID, uint16_t flag, const PB& pb)
	{
		static_assert(std::is_base_of_v<AsioNet::GooglePbLite, PB>, "not a protobuf");

		std::string data = pb.SerializeAsString();

		Header h{msgID,flag};
		memcpy_s(m_buffer, sizeof(m_buffer), &h, sizeof(h));
		memcpy_s(m_buffer+sizeof(h), sizeof(m_buffer) - sizeof(h), data.c_str(), data.length());
		size_t len = data.length() + sizeof(h);
		return netMgr.Send(m_conn, m_buffer, len);
	}

	struct connhandler {
		void operator()(void* cl, AsioNet::NetKey key, std::string, uint16_t) {
			TestClient* pkClient = (TestClient * )cl;
			pkClient->m_conn = key;
			pkClient->DoTest();
		};
	};
	struct disconnhandler {
		void operator()(void* cl, AsioNet::NetKey key, std::string, uint16_t) {
			TestClient* pkClient = (TestClient*)cl;
			memset(pkClient->m_buffer, 0, sizeof(pkClient->m_buffer));
			pkClient->netMgr.Connect("127.0.0.1", 8888, 100);
		};
	};
	void InitRouter()
	{
		m_ed.RegisterConnectHandler<connhandler>(this);
		m_ed.RegisterDisconnectHandler<disconnhandler>(this);
		// RegConnHandler
		// RegDisconHandler
	}

	void DoTest()
	{
		while (m_conn == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		for(int i = 0;;++i) {

			protobuf::DemoPb pb;
			pb.set_a(i);

			SendMsg(1,6, pb);

			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
		//int n = AsioNet::AN_MSG_MAX_SIZE;
		//std::string data;
		//for (int i = 0; i < n; ++i)
		//{
		//	data.push_back(char('a' + i % 26));
		//}

		//int mod = 1'0000;
		//for (int trans = 1; trans <= data.length(); ++trans)
		//{
		//	auto s = data.substr(0, trans);
		//	{
		//		_lock_guard_(lock);
		//		que.push(s);
		//	}
		//	bool succ = netMgr.Send(m_conn, s.c_str(), s.length());
		//	std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//}
	}
private:
	AsioNet::EventDriver m_ed;
	AsioNet::TcpNetMgr netMgr;
	std::atomic<AsioNet::NetKey> m_conn;

	std::mutex lock;
	std::queue<std::string> que;
	char m_buffer[AsioNet::AN_MSG_MAX_SIZE];
};