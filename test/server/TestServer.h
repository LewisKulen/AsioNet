#pragma once

#include "../../src/AsioNet.h"

#include "../../protoc/cpp_all_pb.h"

#include <iostream>

class TestServer {
#include "TestServerRouter.h"
public:
	TestServer() :m_netMgr(8)
	{
		InitRouter();
		// m_netMgr.Serve(&m_ed, "127.0.0.1", 8888);
		// std::cout << "listen at[127.0.0.1:8888]" << std::endl;

		m_netMgr.Serve(&m_ed,"127.0.0.1", 8888,6666);
		std::cout << "listen at[127.0.0.1:8888,6666]" << std::endl;
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
	// AsioNet::TcpNetMgr m_netMgr;
	AsioNet::KcpNetMgr m_netMgr;
	AsioNet::EventDriver m_ed;

};