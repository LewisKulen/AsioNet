#include "./src/TcpNetMgr.h"
#include "./src/test.h"
#include "./src/utils.h"

#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

// #include <boost/thread.hpp>

struct TestServerHandler :public AsioNet::IEventHandler {
	void AcceptHandler(AsioNet::NetKey) {};
	void ConnectHandler(AsioNet::NetKey) {};
	void DisconnectHandler(AsioNet::NetKey) {};
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) {};
	void RecvHandler(AsioNet::NetKey, const std::string&) {};
};

class TestServer :public AsioNet::IEventHandler {
public:
	TestServer() :netMgr(2, this)
	{
		std::thread([this]() {
			while (true) {
				netMgr.Update();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}).detach();
		netMgr.Serve(8888);
		std::cout << "listen at:8888" << std::endl;
	}
	void AcceptHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		auto local = AsioNet::NetKey2ServerKey(k);
		printf("accept[%d]:[%s:%d]\n", local, remote.ip.c_str(), remote.port);
	}
	void ConnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("connect to[%s:%d]\n", remote.ip.c_str(), remote.port);
	}
	void DisconnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("disconnect from[%s:%d]\n", remote.ip.c_str(), remote.port);
	}
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) override {};
	void RecvHandler(AsioNet::NetKey k, const std::string& data) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("recv from[%s:%d]:%s\n", remote.ip.c_str(), remote.port, data.c_str());
		// echo
		printf("echo[%s:%d]:%s\n", remote.ip.c_str(), remote.port, data.c_str());
		bool succ = netMgr.Send(k, data.c_str(), data.length());
		printf("echo succ[%d]\n", succ);
	}
	void ErrorHandler(AsioNet::NetKey, const AsioNet::NetErr& err) override
	{
		std::cout << err.message().c_str() << std::endl;
	}
private:
	AsioNet::TcpNetMgr netMgr;
};

struct TestClient :public AsioNet::IEventHandler {
public:
	TestClient() :netMgr(2, this), m_conn(0)
	{
		std::thread([this]() {
			while (true) {
				netMgr.Update();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}).detach();

		netMgr.Connect("127.0.0.1", 8888, 100);
	}
	void Update()
	{
		while (m_conn == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		for (int i = 1;; ++i)
		{
			auto s = std::to_string(i);
			printf("send len[%d]:data[%s]\n", s.length(), s.c_str());
			bool succ = netMgr.Send(m_conn, s.c_str(), s.length());
			if (!succ)
			{
				printf("send err\n");
			}
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}

	void AcceptHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		auto local = AsioNet::NetKey2ServerKey(k);
		printf("accept[%d]:[%s:%d]\n", local, remote.ip.c_str(), remote.port);
	}
	void ConnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("connect to[%s:%d]\n", remote.ip.c_str(), remote.port);
		m_conn = k;
	}
	void DisconnectHandler(AsioNet::NetKey k) override
	{
		m_conn = AsioNet::INVALID_NET_KEY;
		auto remote = AsioNet::NetKey2Addr(k);
		printf("disconnect from[%s:%d]\n", remote.ip.c_str(), remote.port);
		netMgr.Connect("127.0.0.1", 8888, 100);
	}
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) override {};
	void RecvHandler(AsioNet::NetKey k, const std::string& data) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("recv from[%s:%d]:%s\n", remote.ip.c_str(), remote.port, data.c_str());
		// echo
		// netMgr.Send(k, data.c_str(), data.length());
	}
	void ErrorHandler(AsioNet::NetKey, const AsioNet::NetErr& err) override
	{
		std::cout << err.message().c_str() << std::endl;
	}
private:
	AsioNet::TcpNetMgr netMgr;
	std::atomic<AsioNet::NetKey> m_conn;
};

int main()
{
	// TestServer s;

	TestClient c;
	c.Update();
	return 0;
}