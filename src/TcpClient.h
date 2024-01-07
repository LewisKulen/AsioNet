#pragma once

#include "TcpConn.h"

namespace AsioNet
{
	class TcpClient {
	public:
		TcpClient() = delete;
		TcpClient(io_context& ctx);
		void Connect(std::string addr, unsigned short port);
		void Send(const char* data,unsigned short trans);
		void Close();
	protected:
		void connect_handler(const error_code&);
	private:
		TcpConn conn;
	};
}