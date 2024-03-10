#pragma once

class AcceptHandler {
public:
	void operator()(void* svr, AsioNet::NetKey, std::string, uint16_t);
};

class ConnectHandler {
public:
	void operator()(void* svr, AsioNet::NetKey, std::string, uint16_t);
};

class DisconnectHandler {
public:
	void operator()(void* svr, AsioNet::NetKey, std::string, uint16_t);
};

class ErrorHandler {
public:
	void operator()(void* svr, AsioNet::NetKey key, AsioNet::EventErrCode);
};


class TestRouter {
public:
	void operator()(void* svr, AsioNet::NetKey, const protobuf::DemoPb&);
};