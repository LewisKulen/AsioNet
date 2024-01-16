#include "AsioNetDef.h"
// 接口文档说明

namespace AsioNet
{
    // 思考应该提供什么样的接口给使用者
    struct NetMgr{
        void listen(unsigned short port/*,options*/);
        void connect(std::string ip,unsigned short port/*,options*/);

        bool Send(NetKey,const char* data,size_t trans);

        // update
        void OnRecv(NetKey,const char* data,size_t trans);
        void OnRecv(NetKey,const std::string& data);    // for protobuf

        void OnConnect(NetKey,const std::string& ip,unsigned short port);
        void OnAccept(NetKey,const std::string& ip,unsigned short port);
        void OnDisconnect(NetKey);
    };
    

}
