# AsioNet



- 简介

```c++
包管理工具：vcpkg
项目依赖：vcpkg.json
编译器版本：c++17及以上
    
特性：
    1.x64
    2.跨平台
    3.支持Tcp和Kcp(kcp还在实现中)
    4.封装asio

    
设计原则：
    1.代码是给人看的，因此均使用简单的模板实现
    2.尽量使用面向对象编程，因此Handler被约定成仿函数
    3.现代计算机，性能已经强了很多，不差虚函数/std::function这种开销，适度的抽象，解耦是有必要的
 
/*
Tcp
    TcpNetMgr---------
      | 			|
      v 			v
    TcpServer ----> TcpConn	----> IEventPoller ----> EventDriver ----> Handler
    
    TcpConn
    	实现原理:封装asio
    	
    EventDriver：
    	实现原理:模板+std::function,单线程


Kcp
	KcpSock ----> KcpClient
	   |
	KcpServer 
*/
        	
   
    
测试代码均在src/test文件夹下

```

