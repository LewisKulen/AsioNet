#pragma once

#include "TcpConn.h"

namespace AsioNet
{
	class TcpClient {
	public:
		TcpClient() = delete;
		TcpClient(io_ctx& ctx);
		void Connect(std::string addr, unsigned short port);
		void Send(const char* data, size_t trans);
		void Close();
	protected:
		void connect_handler(const NetErr&);
	private:
		std::shared_ptr<TcpConn> conn;
	};
}