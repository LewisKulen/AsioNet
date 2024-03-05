#pragma once

#include "../src/event/EventDriver.h"
#include "../src/TcpNetMgr.h"

#include "../protoc/cpp_all_pb.h"

class TestServer {
#include "TestServerRouter.h"
public:
	TestServer() :m_netMgr(8, &m_ed)
	{
		InitRouter();
		m_netMgr.Serve(8888);
		std::cout << "listen at:8888" << std::endl;
	}
	void Update()
	{
		while (true) {
			while (m_ed.RunOne());
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	void InitRouter()
	{
		m_ed.RegisterAcceptHandler<AcceptHandler>(this);
		m_ed.RegisterConnectHandler<ConnectHandler>(this);
		m_ed.RegisterDisconnectHandler<DisconnectHandler>(this);
		m_ed.RegisterErrHandler<ErrorHandler>(this);

		m_ed.AddRouter<TestRouter, protobuf::DemoPb>(this,1);
	}

private:
	AsioNet::TcpNetMgr m_netMgr;
	AsioNet::EventDriver m_ed;
};