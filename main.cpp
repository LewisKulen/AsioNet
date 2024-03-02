
#include "./src/kcp/KcpConn.h"

#include "./src/test/TestServer.h"
#include "./src/test/TestClient.h"

int main()
{

	TestClient c;

	//TestServer s;
	//s.Update();


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
