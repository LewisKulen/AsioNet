#include "AsioNetDef.h"

namespace AsioNet
{
	enum AN_EVENT {
		AN_ACCEPT = 0,
		AN_CONNECT = 1,
		AN_DISCONNECT = 2,
		AN_RECV = 3,
	};

	class AN_Event {

	};

	class AcceptEvent {
	public:
	private:
	};
	class ConnectEvent {

	};
	class DisconnectEvent {

	};

	class RecvEvent {

	};

	struct AN_INTERFACE AN_IEventHandler {
		virtual void ProcAccept(const std::string& remote,unsigned short port) = 0;
		virtual void ProcConnect(const std::string& remote, unsigned short port) = 0;
		virtual void ProcDisconnect(const std::string& remote, unsigned short port) = 0;
		virtual void ProcRecv(const char* data,size_t trans) = 0;
		virtual ~AN_IEventHandler() {}
	};

	class AN_Default_EventHandler :public AN_IEventHandler {
	public:

	};
}