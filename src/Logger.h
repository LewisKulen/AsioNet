
#include <iostream>
#include <mutex>

namespace AsioNet
{
	struct ILogger {
		virtual ~ILogger() {}
		virtual void Log(const TcpSock& rkSock, const NetErr& err) = 0;
	};

	class TestLogger :public ILogger , public std::enable_shared_from_this<TestLogger> {
	public:
		void Log(const TcpSock& rkSock, const NetErr& err)
		{
			std::lock_guard<std::mutex> guard(lock);
			std::cout << err.message().c_str() << std::endl;
		}
		void Log(std::string& str)
		{
			std::lock_guard<std::mutex> guard(lock);
			std::cout << str.c_str() << std::endl;
		}
		std::mutex lock;
	};
	
}