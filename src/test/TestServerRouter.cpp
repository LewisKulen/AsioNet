#include "TestServer.h"
#include "./test.h"

#define REFER_TO_TEST_SVR(svr) auto& rkSvr = *(static_cast<TestServer*>(svr));

void TestServer::AcceptHandler::operator()
(void* svr, AsioNet::NetKey key, std::string ip, uint16_t port)
{
	fghtest::log("accept from " + ip + ":" + std::to_string(port));
}

void TestServer::ConnectHandler::operator()
(void* svr, AsioNet::NetKey key, std::string ip, uint16_t port)
{
	fghtest::log("connect to " + ip + ":" + std::to_string(port));
}

void TestServer::DisconnectHandler::operator()
(void* svr, AsioNet::NetKey key, std::string ip, uint16_t port)
{
	fghtest::log("disconnect " + ip + ":" + std::to_string(port));
}

void TestServer::ErrorHandler::operator()
(void* svr, AsioNet::NetKey key, AsioNet::EventErrCode ec)
{
	REFER_TO_TEST_SVR(svr);
	fghtest::log("error:" + std::to_string(int(ec)));
	// rkSvr.m_netMgr.Disconnect(key);
}


void TestServer::TestRouter::operator()
(void* svr, AsioNet::NetKey, const protobuf::DemoPb& pb)
{
	static int cnt = 0;
	fghtest::log(std::to_string(++cnt) + ":" + std::to_string(pb.a()));

}
