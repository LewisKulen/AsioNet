#pragma once

#include "../../src/AsioNet.h"

#include "../../protoc/cpp_all_pb.h"

#include <type_traits>

struct Header {
	uint16_t msgid;
	uint16_t flag;
};

struct TestClient{
public:
	TestClient() :m_netMgr(2), m_conn(0)
	{
		InitRouter();
		memset(m_buffer, 0, sizeof(m_buffer));
		// m_netMgr.Connect(&m_ed, "127.0.0.1", 8888, 100);
		m_netMgr.Connect(&m_ed, "127.0.0.1", 8888, 6666);
	}
	void Update()
	{
		while (true) {
			while (m_ed.RunOne());
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
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
		return m_netMgr.Send(m_conn, m_buffer, len);
	}

	struct connhandler {
		void operator()(void* cl, AsioNet::NetKey key, std::string ip, uint16_t port) {
			printf("connect to:[%s:%d]\n",ip.c_str(),port);
			TestClient* pkClient = (TestClient * )cl;
			pkClient->m_conn = key;
			pkClient->DoTest();
		};
	};
	struct disconnhandler {
		void operator()(void* cl, AsioNet::NetKey key, std::string, uint16_t) {
			TestClient* pkClient = (TestClient*)cl;
			pkClient->m_conn = 0;
			memset(pkClient->m_buffer, 0, sizeof(pkClient->m_buffer));
			// pkClient->m_netMgr.Connect(&(pkClient->m_ed), "127.0.0.1", 8888, 100);
			pkClient->m_netMgr.Connect(&(pkClient->m_ed),"127.0.0.1", 8888, 6666);
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
		for(int i = 0;;++i) {
			if (m_conn == 0) {
				break;
			}

			protobuf::DemoPb pb;
			pb.set_a(i);

			SendMsg(1,6, pb);
			std::cout << "send " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
private:
	AsioNet::EventDriver m_ed;
	// AsioNet::TcpNetMgr m_netMgr;

	AsioNet::KcpNetMgr m_netMgr;

	std::atomic<AsioNet::NetKey> m_conn;

	char m_buffer[AsioNet::AN_MSG_MAX_SIZE];
};