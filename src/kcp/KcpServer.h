#pragma once

#include "./KcpConn.h"
#include "../utils/utils.h"

namespace AsioNet
{
	// ikcp_allocator:可以考虑接管内存管理
	class KcpServer : public std::enable_shared_from_this<KcpServer>
	{
	public:
		KcpServer() = delete;
		KcpServer(const KcpServer&) = delete;
		KcpServer(KcpServer&&) = delete;
		KcpServer& operator=(const KcpServer&) = delete;
		KcpServer& operator=(KcpServer&&) = delete;

		KcpServer(uint32_t id,io_ctx& ctx, IEventPoller* p);

		~KcpServer();

		void Serve(uint16_t port);
		
		bool Write(const char* data, size_t trans);

		KcpKey Key();


	private:
		KcpKey m_key;

	};

}
