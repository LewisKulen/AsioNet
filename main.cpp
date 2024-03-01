#include "./src/TcpNetMgr.h"
#include "./src/test.h"
#include "./src/utils.h"

#include "./src/kcp/KcpConn.h"
#include "./src/event/IEventPoller.h"


#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

class TestServer {
public:
	TestServer() :netMgr(8, this)
	{
		netMgr.Serve(8888);
		std::cout << "listen at:8888" << std::endl;
	}
	void Update()
	{
		while (true) {
			netMgr.Update();

			// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
private:
	AsioNet::TcpNetMgr netMgr;
};

struct TestClient{
public:
	TestClient() :netMgr(2, this), m_conn(0), hasErr(false)
	{
		std::thread([this]() {
			while (true) {
				netMgr.Update();
				// ����ֻ��Ϊ�˲��Է��㣬���⿪�˸��̣߳�ʵ��ʹ��ʱ��Ҫ��ô��
				// ��Ȼ���߳������˻������̵߳��߼���G��
			}
			}).detach();

			netMgr.Connect("127.0.0.1", 8888, 100);
	}
	void DoTest()
	{
		while (m_conn == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		int n = AsioNet::AN_MSG_MAX_SIZE;
		std::string data;
		for (int i = 0; i < n; ++i)
		{
			data.push_back(char('a' + i % 26));
		}

		int mod = 1'0000;
		for (int trans = 1; trans <= data.length(); ++trans)
		{
			auto s = data.substr(0, trans);
			{
				_lock_guard_(lock);
				que.push(s);
			}
			bool succ = netMgr.Send(m_conn, s.c_str(), s.length());
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}
	}
private:
	AsioNet::TcpNetMgr netMgr;
	std::atomic<AsioNet::NetKey> m_conn;

	std::mutex lock;
	std::queue<std::string > que;
};

int main()
{
	// TestServer s;
	// s.Update();
	// TestClient c;
	// c.DoTest();
	// printf("client test finish:succ[%d]\n",!(c.hasErr));
	
	// AsioNet::io_ctx ctx;
	// std::thread th([&ctx]{
	// 	while(true){
	// 		ctx.run();
	// 	}
	// });
	// KcpPoller poller;
	// auto client = std::make_shared<AsioNet::KcpConn>(ctx,&poller);
	// client->Write("12345",5);

	// auto server = std::make_shared<AsioNet::KcpConn>(ctx,&poller);
	
	
	// th.join();
	return 0;
}
