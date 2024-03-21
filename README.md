# AsioNet

## 简介

闲着没事干写的网络库，基于Asio开发，支持TCP和KCP。功能很原始。

-   参考资料
    1.   https://github.com/libinzhangyuan/asio_kcp
    2.   https://pkg.go.dev/github.com/xtaci/kcp-go 
    3.   https://luyuhuang.tech/2020/12/09/kcp.html 
    4.   https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html 

-   设计原则

1.   代码是给人看的，因此均使用简单的模板实现
2.   尽量使用面向对象编程，因此Handler被约定成仿函数

期待你的出宝贵建议~

## 项目构建

-   包管理工具:VCPKG

-   项目依赖:见vcpkg.json

-   语言版本:C++17以上

-   搭建

```c++
1.   vs2022+vcpkg
         vs2022里自带vcpkg，使用vcpkg的manifest模式，vcpkg integrate install 集成到项目，然后编译运行即可
2.   vscode+vcpkg+cmake
         1.下载vscode扩展：C/C++,CMake,CMake Tool
         2.由于我的编译器是mingw，你只需要修改.vscode中的部分就可以
```

## 架构简图

```c++

/*
Tcp
    TcpNetMgr---------
      | 			|
      v 			v
    TcpServer ----> TcpConn	----> IEventPoller ----> EventDriver（自己实现的一个，你可以自己实现）
    
Kcp结构同上
*/

```

## 已知问题

1.   kcp的断连无法立刻知晓，需要新增心跳
